// The vocal chain modules: input conditioning, colour, doubler, modulation, echo.
#pragma once

#include "Dsp.h"
#include <array>

namespace dg
{
inline void panGains (double pan, double& l, double& r)   // pan -1..1, constant power
{
    const double a = (clampd (pan, -1, 1) + 1.0) * kPi * 0.25;
    l = std::cos (a); r = std::sin (a);
}

// ---------------------------------------------------------------- Input
// Gain, low cut, one-knob opto-style compressor, split-band de-esser, drive.
class InputStage
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        for (int c = 0; c < 2; ++c) { split[c].set (fs, 5200); dc[c].set (fs, 10); }
        sibBp.bandpass (fs, 7000, 0.9);
        sibEnv.set (fs, 0.8, 60); bodyEnv.set (fs, 4, 120);
        detEnv.set (fs, 2, 60);
        grFast.setTime (fs, 0.07); grSlow.setTime (fs, 1.1);
        fuzzLp[0].lowpass (fs, 4200, 0.7); fuzzLp[1].lowpass (fs, 4200, 0.7);
        reset();
    }

    void reset()
    {
        for (int c = 0; c < 2; ++c) { hp[c].reset(); split[c].reset(); dc[c].reset(); fuzzLp[c].reset(); }
        sibBp.reset(); sibEnv.reset(); bodyEnv.reset(); detEnv.reset();
        grFast.reset(); grSlow.reset(); deessGr = 0;
    }

    void configure (double gainDb, double lowCutHz, double comp, double deess, double drive, int driveType)
    {
        gain = dbToGain (gainDb);
        for (auto& h : hp) h.highpass (fs, lowCutHz, 0.71);
        compAmt = clampd (comp, 0, 1);
        thr = -8.0 - 30.0 * compAmt;
        ratio = 1.5 + 6.0 * compAmt;
        makeup = dbToGain (0.45 * -thr * (1.0 - 1.0 / ratio) * compAmt);
        deessAmt = clampd (deess, 0, 1);
        type = driveType;
        driveAmt = clampd (drive, 0, 1);
        if (type == 2) k = dbToGain (6.0 + driveAmt * 40.0);
        else k = dbToGain (driveAmt * 24.0);
    }

    inline void process (double& l, double& r)
    {
        l = hp[0].process (l * gain);
        r = hp[1].process (r * gain);

        // compressor: stereo-linked, two-stage release (fast + slow) like an optical cell
        if (compAmt > 0.001)
        {
            const double lvl = gainToDb (detEnv.process (std::max (std::fabs (l), std::fabs (r))));
            const double over = lvl - thr, knee = 8.0;
            double gr = 0;
            if (over > knee * 0.5) gr = over * (1.0 - 1.0 / ratio);
            else if (over > -knee * 0.5) { const double t = over + knee * 0.5; gr = (1.0 - 1.0 / ratio) * t * t / (2.0 * knee); }
            const double f = gr > grFast.z ? (grFast.z = grFast.z + (gr - grFast.z) * 0.02) : grFast.process (gr);
            const double s = gr > grSlow.z ? (grSlow.z = grSlow.z + (gr - grSlow.z) * 0.004) : grSlow.process (gr);
            compGr = 0.65 * f + 0.35 * s;
            const double g = dbToGain (-compGr) * makeup;
            l *= g; r *= g;
        }
        else compGr = 0;

        // de-esser: reduce only the band above ~5 kHz when the sibilance band jumps above the body
        if (deessAmt > 0.001)
        {
            const double m = 0.5 * (l + r);
            const double s = sibEnv.process (sibBp.process (m));
            const double b = bodyEnv.process (m);
            const double over = gainToDb (s) - gainToDb (b) + 14.0 * deessAmt - 4.0;
            deessGr = clampd (over * 1.2, 0.0, 4.0 + 16.0 * deessAmt);
            const double hg = dbToGain (-deessGr);
            const double lo0 = split[0].process (l), lo1 = split[1].process (r);
            l = lo0 + (l - lo0) * hg;
            r = lo1 + (r - lo1) * hg;
        }
        else deessGr = 0;

        if (driveAmt > 0.001 || type == 2)
        {
            l = sat (l, 0); r = sat (r, 1);
        }
    }

    double compGr = 0, deessGr = 0;

private:
    inline double sat (double x, int c)
    {
        switch (type)
        {
            case 1:   // tube: asymmetric, adds even harmonics
            {
                const double b = 0.3, tb = std::tanh (b);
                return dc[c].process ((std::tanh (k * x + b) - tb) / (k * (1.0 - tb * tb)));
            }
            case 2:   // fuzz: high gain, band-limited, blends in with the knob
            {
                const double f = fuzzLp[c].process (std::tanh (k * x)) * 0.35;
                return x + (f - x) * std::min (1.0, driveAmt * 3.0);
            }
            default:  // tape: symmetric soft saturation, unity for small signals
                return std::tanh (k * x) / k;
        }
    }

    double fs = 48000, gain = 1, compAmt = 0, thr = -10, ratio = 2, makeup = 1, deessAmt = 0, driveAmt = 0, k = 1;
    int type = 0;
    Biquad hp[2], sibBp, fuzzLp[2];
    OnePoleLP split[2], grFast, grSlow;
    DcBlocker dc[2];
    EnvFollower sibEnv, bodyEnv, detEnv;
};

// ---------------------------------------------------------------- Colour
// Band-limiting with resonance (telephone, AM radio, old mic), crush, cassette wobble, noise.
class ColourStage
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        for (auto& d : wob) d.allocate ((int) (0.03 * fs));
        wowDrift.set (fs, 0.9, 0xC0FFEEu);
        flutter.set (fs, 6.3);
        hissHp.highpass (fs, 1500, 0.7);
        reset();
    }
    void reset()
    {
        for (int c = 0; c < 2; ++c) { lo[c].reset(); hi[c].reset(); wob[c].clear(); held[c] = 0; }
        hissHp.reset(); holdPhase = 0;
    }

    void configure (double lowCut, double highCut, double reso, double crush, double wobble, double noise, double mix)
    {
        const double q = 0.707 + clampd (reso, 0, 1) * 4.5;
        for (int c = 0; c < 2; ++c)
        {
            if (lowCut > 21) lo[c].highpass (fs, lowCut, q); else lo[c].bypass();
            if (highCut < 19900) hi[c].lowpass (fs, highCut, q); else hi[c].bypass();
        }
        crushAmt = clampd (crush, 0, 1);
        levels = std::pow (2.0, 15.0 - crushAmt * 11.0);
        holdStep = 1.0 / (1.0 + crushAmt * crushAmt * 11.0);
        wobAmt = clampd (wobble, 0, 1);
        noiseAmt = clampd (noise, 0, 1);
        mixAmt = clampd (mix, 0, 1);
    }

    inline void process (double& l, double& r)
    {
        double x[2] = { l, r };
        const double dry[2] = { l, r };
        bool hold = false;
        if (crushAmt > 0.001)
        {
            holdPhase += holdStep;
            if (holdPhase >= 1.0) { holdPhase -= 1.0; hold = true; }
        }
        double wobD = 0;
        if (wobAmt > 0.001)
        {
            flutter.tick();
            wobD = (6.0 + wobAmt * (wowDrift.process() * 2.2 + flutter.sine() * 0.18)) * 0.001 * fs;
        }
        const double hiss = noiseAmt > 0.001 ? hissHp.process (rng.bi()) * noiseAmt * 0.012 : 0.0;
        double crackle = 0;
        if (noiseAmt > 0.001 && rng.uni() < noiseAmt * 6.0 / fs) crackle = rng.bi() * 0.06 * noiseAmt;

        for (int c = 0; c < 2; ++c)
        {
            double v = hi[c].process (lo[c].process (x[c]));
            if (crushAmt > 0.001)
            {
                if (hold) held[c] = std::round (v * levels) / levels;
                v = held[c];
            }
            if (wobAmt > 0.001) { wob[c].push (v); v = wob[c].tapHermite (wobD); }
            v += hiss + crackle;
            x[c] = dry[c] + (v - dry[c]) * mixAmt;
        }
        l = x[0]; r = x[1];
    }

private:
    double fs = 48000, crushAmt = 0, levels = 32768, holdStep = 1, holdPhase = 0, wobAmt = 0, noiseAmt = 0, mixAmt = 1;
    Biquad lo[2], hi[2], hissHp;
    DelayLine wob[2];
    double held[2] {};
    Drift wowDrift;
    Lfo flutter;
    Rng rng { 0x1234567u };
};

// ---------------------------------------------------------------- Double / harmony voices
class Doubler
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        for (int i = 0; i < kVoices; ++i)
        {
            voices[(size_t) i].ps.prepare (fs, 50.0, 110.0);
            voices[(size_t) i].drift.set (fs, 0.35 + 0.1 * i, 0xD0u + (uint32_t) i * 31u);
        }
        reset();
    }
    void reset() { for (auto& v : voices) v.ps.clear(); }

    void configure (int mode, double detune, double delay, double width, double mix)
    {
        const double c = 3.0 + detune * 22.0;                // cents
        const double d = delay;                               // 0..1
        const double w = clampd (width, 0, 1);
        auto st = [] (double cents) { return std::pow (2.0, cents / 1200.0); };
        active = 2;
        for (auto& v : voices) v.gain = 0;
        auto setV = [&] (int i, double ratio, double ms, double pan, double g)
        {
            auto& v = voices[(size_t) i];
            v.ratio = ratio; v.delay = ms * 0.001 * fs; panGains (pan, v.gl, v.gr); v.gain = g;
        };
        switch (mode)
        {
            case 1: setV (0, 2.0 * st (c * 0.3), 8 + d * 25, -w * 0.6, 0.6); setV (1, 2.0 * st (-c * 0.3), 14 + d * 30, w * 0.6, 0.6); break;
            case 2: setV (0, 0.5 * st (c * 0.3), 8 + d * 25, -w * 0.5, 0.7); setV (1, 0.5 * st (-c * 0.3), 14 + d * 30, w * 0.5, 0.7); break;
            case 3: setV (0, 1.4983 * st (c * 0.3), 8 + d * 25, -w * 0.6, 0.6); setV (1, 1.4983 * st (-c * 0.3), 14 + d * 30, w * 0.6, 0.6); break;
            case 4: setV (0, 2.0, 10 + d * 25, -w, 0.55); setV (1, 0.5, 16 + d * 30, w, 0.6); break;
            case 5:
                active = 4;
                setV (0, st (c), 14 + d * 40, -w, 0.45);
                setV (1, st (-c * 0.8), 22 + d * 50, -w * 0.35, 0.45);
                setV (2, st (c * 0.55), 30 + d * 45, w * 0.35, 0.45);
                setV (3, st (-c), 18 + d * 55, w, 0.45);
                break;
            default: setV (0, st (c), 12 + d * 30, -w, 0.7); setV (1, st (-c), 19 + d * 38, w, 0.7); break;
        }
        detuneCents = c;
        mixAmt = clampd (mix, 0, 1);
    }

    inline void process (double& l, double& r)
    {
        const double m = 0.5 * (l + r);
        double sl = 0, sr = 0;
        for (int i = 0; i < active; ++i)
        {
            auto& v = voices[(size_t) i];
            // a slow random wander of the detune keeps the double from sounding like a static chorus
            const double wander = 1.0 + v.drift.process() * detuneCents * (0.25 * 0.000577623);   // ~2^(cents/1200)
            const double y = v.ps.process (m, v.ratio * wander, v.delay) * v.gain;
            sl += y * v.gl; sr += y * v.gr;
        }
        l += sl * mixAmt; r += sr * mixAmt;
    }

private:
    static constexpr int kVoices = 4;
    struct Voice
    {
        PitchShifter ps;
        Drift drift;
        double ratio = 1, delay = 0, gl = 0.7, gr = 0.7, gain = 0;
    };
    double fs = 48000, mixAmt = 0.5, detuneCents = 10;
    int active = 2;
    std::array<Voice, kVoices> voices;
};

// ---------------------------------------------------------------- Modulation
class ModStage
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        for (auto& d : line) d.allocate ((int) (0.05 * fs));
        drift.set (fs, 1.2, 0xBEEFu);
        reset();
    }
    void reset() { for (auto& d : line) d.clear(); fb[0] = fb[1] = 0; }

    void configure (int m, double rateHz, double depth, double mix)
    {
        mode = m;
        lfo.set (fs, rateHz);
        lfo2.set (fs, rateHz * 8.5 + 3.0);
        dep = clampd (depth, 0, 1);
        mixAmt = clampd (mix, 0, 1);
        if (mode == 4) drift.set (fs, std::max (0.2, rateHz * 0.8), 0xBEEFu);
    }

    inline void process (double& l, double& r)
    {
        const double ms = 0.001 * fs;
        double x[2] = { l, r }, wet[2] = { 0, 0 };
        lfo.tick(); lfo2.tick();
        const double dr = mode == 4 ? drift.process() : 0.0;
        for (int c = 0; c < 2; ++c)
        {
            auto& d = line[c];
            const double off = c * 0.25;
            switch (mode)
            {
                case 1:   // ensemble: three phases, slow + fast LFO, like a string machine
                {
                    d.push (x[c]);
                    for (int t = 0; t < 3; ++t)
                    {
                        const double o = t / 3.0 + c / 6.0;
                        const double dl = 9.0 * ms + dep * (lfo.sineAt (o) * 3.0 + lfo2.sineAt (o) * 0.35) * ms;
                        wet[c] += d.tapHermite (dl) / 3.0;
                    }
                    break;
                }
                case 2:   // vibrato: pitch only
                    d.push (x[c]);
                    wet[c] = d.tapHermite ((5.0 + dep * 3.0 * lfo.sine()) * ms);
                    break;
                case 3:   // flanger with feedback
                {
                    d.push (x[c] + fb[c] * 0.6);
                    wet[c] = d.tapHermite ((0.4 + (1.0 + lfo.triAt (off)) * (0.2 + dep * 2.6)) * ms);
                    fb[c] = wet[c];
                    break;
                }
                case 4:   // warble: tape wow
                    d.push (x[c]);
                    wet[c] = d.tapHermite ((7.0 + dep * 4.0 * dr) * ms);
                    break;
                default:  // chorus: two voices per side
                    d.push (x[c]);
                    wet[c] = 0.5 * (d.tapHermite ((14.0 + dep * 6.0 * lfo.sineAt (off)) * ms)
                                  + d.tapHermite ((19.0 + dep * 5.0 * lfo.sineAt (off + 0.5)) * ms));
                    break;
            }
        }
        if (mode == 2 || mode == 4)
        {
            l = x[0] + (wet[0] - x[0]) * mixAmt;
            r = x[1] + (wet[1] - x[1]) * mixAmt;
        }
        else
        {
            const double norm = 1.0 / (1.0 + 0.5 * mixAmt);
            l = (x[0] + wet[0] * mixAmt) * norm;
            r = (x[1] + wet[1] * mixAmt) * norm;
        }
    }

private:
    double fs = 48000, dep = 0.5, mixAmt = 0.5;
    int mode = 0;
    DelayLine line[2];
    double fb[2] {};
    Lfo lfo, lfo2;
    Drift drift;
};

// ---------------------------------------------------------------- Echo
// Tape, Analog (BBD-style), Ping-Pong and Diffuse. Returns the wet signal only.
class Echo
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        for (auto& d : line) d.allocate ((int) (2.6 * fs) + 64);
        for (int c = 0; c < 2; ++c)
            for (int k = 0; k < 3; ++k)
                diff[c][k].allocate ((int) (kDiffMs[k] * 0.001 * fs * 1.2) + 4);
        wowDrift.set (fs, 0.7, 0xABCDu);
        flutter.set (fs, 5.7);
        timeL.prepare (fs, 0.18, 0.38 * fs);
        timeR.prepare (fs, 0.18, 0.38 * fs);
        reset();
    }
    void reset()
    {
        for (int c = 0; c < 2; ++c)
        {
            line[c].clear(); lp[c].reset(); lp2[c].reset(); hp[c].reset(); y[c] = 0;
            for (auto& a : diff[c]) a.clear();
        }
    }

    void configure (int t, double timeMs, double feedback, double tone, double wow, double drive, double width)
    {
        if (t != type) reset();
        type = t;
        const double base = clampd (timeMs, 10, 2000) * 0.001 * fs;
        const double w = clampd (width, 0, 1);
        if (type == 2) { timeL.set (base); timeR.set (base); }
        else { timeL.set (base); timeR.set (base * (1.0 + 0.16 * w)); }
        fbGain = clampd (feedback, 0, 1) * 1.06;
        const double fc = 1500.0 * std::pow (2.0, clampd (tone, 0, 1) * 3.6);
        for (int c = 0; c < 2; ++c)
        {
            lp[c].lowpass (fs, type == 1 ? fc * 0.55 : fc, 0.6);
            lp2[c].lowpass (fs, type == 1 ? 3600 : 18000, 0.65);
            hp[c].highpass (fs, type == 1 ? 160 : 100, 0.6);
            for (int k = 0; k < 3; ++k) diff[c][k].len = kDiffMs[k] * 0.001 * fs * (c ? 1.07 : 1.0);
        }
        wowAmt = clampd (wow, 0, 1);
        satK = 1.0 + clampd (drive, 0, 1) * (type == 1 ? 2.0 : 4.0);
        widthAmt = w;
    }

    inline void process (double inL, double inR, double& outL, double& outR)
    {
        flutter.tick();
        const double dL = timeL.next(), dR = timeR.next();
        const double wobble = wowAmt * (type == 1 ? 0.25 * flutter.triAt (0) : (wowDrift.process() * 1.6 + flutter.sine() * 0.12));
        const double modS = wobble * 0.001 * fs;

        const double rL = line[0].tapHermite (dL + modS);
        const double rR = line[1].tapHermite (dR + modS * (type == 2 ? 1.0 : 0.9));
        y[0] = loop (rL, 0);
        y[1] = loop (rR, 1);

        if (type == 2)   // ping-pong: mono in on the left, repeats bounce across
        {
            line[0].push ((inL + inR) * 0.5 + y[1] * fbGain);
            line[1].push (y[0] * fbGain);
            const double m = 0.5 * (rL + rR), s = 0.5 * (rL - rR) * widthAmt;
            outL = m + s; outR = m - s;
        }
        else
        {
            line[0].push (inL + y[0] * fbGain);
            line[1].push (inR + y[1] * fbGain);
            outL = rL; outR = rR;
        }
    }

private:
    inline double loop (double v, int c)
    {
        v = hp[c].process (lp2[c].process (lp[c].process (v)));
        if (type == 3) for (auto& a : diff[c]) v = a.process (v, 0.62);
        return std::tanh (v * satK) / satK;
    }

    static constexpr double kDiffMs[3] = { 7.1, 11.3, 17.9 };
    double fs = 48000, fbGain = 0.3, wowAmt = 0, satK = 1, widthAmt = 0.5;
    int type = 0;
    DelayLine line[2];
    Biquad lp[2], lp2[2], hp[2];
    Allpass diff[2][3];
    Smoothed timeL, timeR;
    Drift wowDrift;
    Lfo flutter;
    double y[2] {};
};
} // namespace dg
