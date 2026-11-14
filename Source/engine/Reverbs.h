// The reverb algorithms. All are stereo in / stereo out, wet only. Pre-delay, low cut,
// ducking and mix are handled by the engine around them.
#pragma once

#include "Dsp.h"
#include <array>

namespace dg
{
// ---------------------------------------------------------------- Feedback delay network
// 8 lines, Householder feedback matrix, per-line damping and modulation, input diffusion.
// Used for Hall, Cathedral, Shimmer, Wash, and as the source of the Reverse reverb.
class Fdn
{
public:
    struct Config
    {
        double size = 1.0;        // delay length scale (0.4 .. 3)
        double rt60 = 2.5;        // seconds
        double dampHz = 6000;     // per-line high damping
        double modDepth = 0.3;    // 0..1
        double modRate = 0.6;     // Hz
        double diffusion = 0.6;   // 0..1, input diffuser strength
        double shimmer = 0.0;     // 0..1, octave-up feedback amount
        double fuzz = 0.0;        // 0..1, drive into the tank (Wash)
        bool randomMod = false;   // wandering instead of sine modulation (Wash)
    };

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        const double r = fs / 48000.0;
        const int maxLine = (int) (kLenMs[N - 1] * 0.001 * fs * kMaxSize) + (int) (40 * r) + 8;
        for (int i = 0; i < N; ++i)
        {
            lines[(size_t) i].allocate (maxLine);
            lfo[(size_t) i].setPhase (i * 0.137);
            drift[(size_t) i].set (fs, 0.4 + 0.07 * i, 0x51u + (uint32_t) i * 101u);
        }
        for (int c = 0; c < 2; ++c)
            for (int k = 0; k < kDiff; ++k)
            {
                auto& ap = diff[(size_t) c][(size_t) k];
                ap.allocate ((int) (kDiffMs[c][k] * 0.001 * fs * 2.2) + 4);
            }
        shifter.prepare (fs, 60.0, 5.0);
        shimLp.lowpass (fs, 5500, 0.6);
        shimHp.highpass (fs, 300, 0.6);
        fuzzLp.lowpass (fs, 3800, 0.6);
        configure (cfg);
        clear();
    }

    void clear()
    {
        for (auto& l : lines) l.clear();
        for (auto& d : damp) d.reset();
        for (auto& ch : diff) for (auto& ap : ch) ap.clear();
        shifter.clear(); shimLp.reset(); shimHp.reset(); fuzzLp.reset();
        std::fill (fb.begin(), fb.end(), 0.0);
        shimState = 0;
    }

    void configure (const Config& c)
    {
        cfg = c;
        const double size = clampd (c.size, 0.3, kMaxSize);
        for (int i = 0; i < N; ++i)
        {
            len[(size_t) i] = kLenMs[i] * 0.001 * fs * size;
            const double loopSec = len[(size_t) i] / fs;
            gain[(size_t) i] = std::pow (10.0, -3.0 * loopSec / std::max (0.1, c.rt60));
            damp[(size_t) i].set (fs, c.dampHz * (1.0 - 0.03 * i));
            lfo[(size_t) i].set (fs, c.modRate * (0.75 + 0.09 * i));
        }
        modSamples = c.modDepth * 14.0 * fs / 48000.0 * std::sqrt (size);
        const double g = 0.45 + 0.3 * clampd (c.diffusion, 0, 1);
        diffG = g;
        for (int ch = 0; ch < 2; ++ch)
            for (int k = 0; k < kDiff; ++k)
                diff[(size_t) ch][(size_t) k].len = std::max (2.0, kDiffMs[ch][k] * 0.001 * fs * (0.6 + 0.6 * std::min (size, 2.0)));
        fuzzGain = 1.0 + c.fuzz * 25.0;
    }

    inline void process (double inL, double inR, double& outL, double& outR)
    {
        if (cfg.fuzz > 0.001)
        {
            inL = fuzzLp.process (std::tanh ((inL + inR) * 0.5 * fuzzGain)) * (0.18 + 0.5 / std::sqrt (fuzzGain));
            inR = inL;
        }
        for (int k = 0; k < kDiff; ++k)
        {
            inL = diff[0][(size_t) k].process (inL, diffG);
            inR = diff[1][(size_t) k].process (inR, diffG);
        }

        std::array<double, N> o {};
        double sum = 0;
        for (int i = 0; i < N; ++i)
        {
            auto& l = lines[(size_t) i];
            const double m = cfg.randomMod ? drift[(size_t) i].process() * 0.6 : lfo[(size_t) i].sine();
            lfo[(size_t) i].tick();
            o[(size_t) i] = l.tapHermite (len[(size_t) i] + modSamples * (1.0 + m));
            sum += o[(size_t) i];
        }

        // shimmer: octave-up copy of the tank fed back in
        double shim = 0;
        if (cfg.shimmer > 0.001)
        {
            shimState = shimHp.process (shimLp.process (shifter.process (sum * 0.25, 2.0, 4.0)));
            shim = shimState * cfg.shimmer * 0.55;
        }

        const double h = sum * (2.0 / N);
        outL = outR = 0;
        for (int i = 0; i < N; ++i)
        {
            const double y = damp[(size_t) i].process (o[(size_t) i] - h) * gain[(size_t) i];
            double in = y + ((i & 1) ? inR : inL) * kInGain + (((i >> 1) & 1) ? -shim : shim);
            if (cfg.shimmer > 0.001 || cfg.fuzz > 0.001) in = 2.0 * std::tanh (in * 0.5);
            lines[(size_t) i].push (in);
            outL += o[(size_t) i] * kSignL[i];
            outR += o[(size_t) i] * kSignR[i];
        }
        outL *= 0.42; outR *= 0.42;
    }

private:
    static constexpr int N = 8;
    static constexpr int kDiff = 4;
    static constexpr double kMaxSize = 3.0;
    static constexpr double kInGain = 0.5;
    static constexpr double kLenMs[N] = { 31.3, 37.9, 41.7, 47.3, 53.1, 61.7, 67.9, 79.3 };
    static constexpr double kDiffMs[2][kDiff] = { { 4.77, 3.59, 12.73, 9.31 }, { 5.13, 3.27, 11.91, 8.93 } };
    static constexpr double kSignL[N] = { 1, -1, 1, 1, -1, 1, -1, 1 };
    static constexpr double kSignR[N] = { 1, 1, -1, 1, 1, -1, -1, -1 };

    double fs = 48000;
    Config cfg;
    std::array<DelayLine, N> lines;
    std::array<OnePoleLP, N> damp;
    std::array<Lfo, N> lfo;
    std::array<Drift, N> drift;
    std::array<double, N> len {}, gain {}, fb {};
    std::array<std::array<Allpass, kDiff>, 2> diff;
    double diffG = 0.6, modSamples = 4, fuzzGain = 1, shimState = 0;
    PitchShifter shifter;
    Biquad shimLp, shimHp, fuzzLp;
};

// ---------------------------------------------------------------- Plate
// Figure-of-eight plate tank after Dattorro (1997): four input diffusers, two cross-coupled
// branches of modulated allpass -> delay -> damping -> allpass -> delay.
class Plate
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        const double r = fs / 29761.0 * kMaxSize;
        for (int i = 0; i < 4; ++i) inAp[(size_t) i].allocate ((int) (kIn[i] * r) + 4);
        apA1.allocate ((int) ((672 + 40) * r)); apB1.allocate ((int) ((908 + 40) * r));
        dA1.allocate ((int) (4453 * r) + 4); dA2.allocate ((int) (3720 * r) + 4);
        dB1.allocate ((int) (4217 * r) + 4); dB2.allocate ((int) (3163 * r) + 4);
        apA2.allocate ((int) (1800 * r) + 4); apB2.allocate ((int) (2656 * r) + 4);
        lfoA.set (fs, 0.9); lfoB.set (fs, 0.73); lfoB.setPhase (0.25);
        configure (2.0, 0.6, 7000, 0.3, 0.5);
        clear();
    }

    void clear()
    {
        for (auto& a : inAp) a.clear();
        apA1.clear(); apB1.clear(); apA2.clear(); apB2.clear();
        dA1.clear(); dA2.clear(); dB1.clear(); dB2.clear();
        bw.reset(); dampA.reset(); dampB.reset();
        fbA = fbB = 0;
    }

    void configure (double rt60, double size, double dampHz, double mod, double diffusion)
    {
        scale = fs / 29761.0 * (0.55 + 0.9 * clampd (size, 0, 1));
        for (int i = 0; i < 4; ++i) inAp[(size_t) i].len = std::max (2.0, kIn[i] * scale);
        apA1.len = 672 * scale; apB1.len = 908 * scale;
        apA2.len = std::max (2.0, 1800 * scale); apB2.len = std::max (2.0, 2656 * scale);
        lA1 = (int) (4453 * scale); lA2 = (int) (3720 * scale); lB1 = (int) (4217 * scale); lB2 = (int) (3163 * scale);
        const double halfLoop = (672 + 4453 + 1800 + 3720) * scale / fs;
        decay = clampd (std::pow (10.0, -3.0 * halfLoop / std::max (0.1, rt60)), 0, 0.995);
        dampA.set (fs, dampHz); dampB.set (fs, dampHz);
        bw.set (fs, std::min (fs * 0.45, dampHz * 2.0));
        modS = mod * 14.0 * fs / 29761.0;
        inDiff1 = 0.55 + 0.25 * clampd (diffusion, 0, 1);
        inDiff2 = 0.45 + 0.2 * clampd (diffusion, 0, 1);
    }

    inline void process (double inL, double inR, double& outL, double& outR)
    {
        double x = bw.process ((inL + inR) * 0.5);
        x = inAp[0].process (x, inDiff1);
        x = inAp[1].process (x, inDiff1);
        x = inAp[2].process (x, inDiff2);
        x = inAp[3].process (x, inDiff2);

        // branch A
        double a = x + fbB * decay;
        a = apA1.process (a, -0.7, modS * lfoA.sine());
        dA1.push (a);
        double ta = dampA.process (dA1.tap (lA1 - 1)) * decay;
        ta = apA2.process (ta, 0.5);
        dA2.push (ta);
        const double nextA = dA2.tap (lA2 - 1);

        // branch B
        double b = x + fbA * decay;
        b = apB1.process (b, -0.7, modS * lfoB.sine());
        dB1.push (b);
        double tb = dampB.process (dB1.tap (lB1 - 1)) * decay;
        tb = apB2.process (tb, 0.5);
        dB2.push (tb);
        const double nextB = dB2.tap (lB2 - 1);

        fbA = nextA; fbB = nextB;
        lfoA.tick(); lfoB.tick();

        auto t = [this] (int samples) { return std::max (0, (int) (samples * scale)); };
        outL = dB1.tap (t (266)) + dB1.tap (t (2974)) - apB2.line.tap (t (1913)) + dB2.tap (t (1996))
             - dA1.tap (t (1990)) - apA2.line.tap (t (187)) - dA2.tap (t (1066));
        outR = dA1.tap (t (353)) + dA1.tap (t (3627)) - apA2.line.tap (t (1228)) + dA2.tap (t (2673))
             - dB1.tap (t (2111)) - apB2.line.tap (t (335)) - dB2.tap (t (121));
        outL *= 0.5; outR *= 0.5;
    }

private:
    static constexpr double kMaxSize = 1.5;
    static constexpr double kIn[4] = { 142, 107, 379, 277 };
    double fs = 48000, scale = 1.6, decay = 0.5, modS = 8, inDiff1 = 0.75, inDiff2 = 0.625;
    std::array<Allpass, 4> inAp;
    Allpass apA1, apB1, apA2, apB2;
    DelayLine dA1, dA2, dB1, dB2;
    int lA1 = 4453, lA2 = 3720, lB1 = 4217, lB2 = 3163;
    OnePoleLP bw, dampA, dampB;
    Lfo lfoA, lfoB;
    double fbA = 0, fbB = 0;
};

// ---------------------------------------------------------------- Spring
// Spring tank model shared with Ferro Echo: dispersive stretched-allpass chains in feedback
// loops give the descending chirp. Two tanks with different spring lengths make it stereo.
class SpringTank
{
public:
    void prepare (double sampleRate, int variant)
    {
        fs = sampleRate;
        const double r = fs / 48000.0;
        const double vScale = variant == 0 ? 1.0 : 1.083;
        static const double lenMs[kSprings] = { 33.7, 39.1, 44.9 };
        for (int s = 0; s < kSprings; ++s)
        {
            auto& sp = springs[(size_t) s];
            sp.len = std::max (8, (int) (lenMs[s] * vScale * 0.001 * fs) - kAllpasses * kStretch);
            sp.delay.allocate (sp.len + 64);
            for (auto& ap : sp.ap) { ap.allocate ((int) std::ceil (kStretch * r) + 2); ap.k = std::max (1, (int) std::lround (kStretch * r)); }
            sp.damp.set (fs, 3800.0 - 300.0 * s);
            sp.mod.set (fs, 0.7 + 0.3 * s, 0xA5u + (uint32_t) s * 77u + (uint32_t) variant * 1234u);
        }
        inHp.highpass (fs, 120.0, 0.6);
        inLp.lowpass (fs, 4800.0, 0.6);
        outHp.highpass (fs, 140.0, 0.7);
        outPeak.peak (fs, 1800.0, 1.2, 2.5);   // pickup resonance: the "boing" colour
        configure (2.6, 0.5, 4500);
        clear();
    }

    void clear()
    {
        for (auto& sp : springs)
        {
            sp.delay.clear();
            for (auto& ap : sp.ap) ap.clear();
            sp.damp.reset(); sp.fb = 0;
        }
        inHp.reset(); inLp.reset(); outHp.reset(); outLp.reset(); outPeak.reset();
    }

    void configure (double rt60, double drip, double toneHz)
    {
        for (int s = 0; s < kSprings; ++s)
        {
            auto& sp = springs[(size_t) s];
            sp.a = 0.45 + 0.3 * clampd (drip, 0, 1) + 0.03 * (s - 1);
            const double loopSec = (sp.len + kAllpasses * kStretch * 1.6) / fs;
            sp.g = std::min (0.97, std::pow (10.0, -3.0 * loopSec / std::max (0.2, rt60)));
        }
        outLp.lowpass (fs, toneHz, 0.75);
    }

    inline double process (double x)
    {
        double in = inLp.process (inHp.process (x));
        in = std::tanh (in * 1.4) / 1.4;
        double out = 0;
        for (int s = 0; s < kSprings; ++s)
        {
            auto& sp = springs[(size_t) s];
            double v = in * (s == 1 ? -1.0 : 1.0) + sp.fb;
            for (auto& ap : sp.ap) v = ap.process (v, sp.a);
            sp.delay.push (v);
            const double d = sp.len * (1.0 + 0.0009 * sp.mod.process());
            const double y = sp.delay.tapLin (std::max (1.0, d));
            sp.fb = sp.damp.process (y) * sp.g;
            out += y;
        }
        return outPeak.process (outLp.process (outHp.process (out))) * 0.55;
    }

private:
    static constexpr int kSprings = 3;
    static constexpr int kAllpasses = 24;
    static constexpr int kStretch = 2;

    struct StretchedAllpass
    {
        std::vector<double> xb, yb; int w = 0, k = 2, n = 4;
        void allocate (int len) { n = len; xb.assign ((size_t) n, 0.0); yb.assign ((size_t) n, 0.0); w = 0; }
        void clear() { std::fill (xb.begin(), xb.end(), 0.0); std::fill (yb.begin(), yb.end(), 0.0); }
        inline double process (double x, double a)
        {
            int r = w - k; if (r < 0) r += n;
            const double y = a * x + xb[(size_t) r] - a * yb[(size_t) r];
            xb[(size_t) w] = x; yb[(size_t) w] = y;
            if (++w >= n) w = 0;
            return y;
        }
    };

    struct Spring
    {
        std::array<StretchedAllpass, kAllpasses> ap;
        DelayLine delay;
        OnePoleLP damp;
        Drift mod;
        int len = 1000;
        double a = 0.6, g = 0.8, fb = 0;
    };

    double fs = 48000;
    std::array<Spring, kSprings> springs;
    Biquad inHp, inLp, outHp, outLp, outPeak;
};

// ---------------------------------------------------------------- Reverse
// The input goes into a hall; the hall's output is cut into segments, and each segment is
// played backwards under a Hann window (two readers half a segment apart), so tails swell
// up into each phrase instead of decaying from it.
class ReverseVerb
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        hall.prepare (fs);
        maxSeg = (int) (1.3 * fs);
        ring.allocate (2 * maxSeg + 8);
        ringR.allocate (2 * maxSeg + 8);
        configure (2.0, 0.5, 6000, 0.5);
        clear();
    }
    void clear() { hall.clear(); ring.clear(); ringR.clear(); j = 0; }

    void configure (double rt60, double length, double dampHz, double mod)
    {
        Fdn::Config c;
        c.size = 0.9; c.rt60 = rt60; c.dampHz = dampHz; c.modDepth = mod; c.diffusion = 0.8;
        hall.configure (c);
        pendingSeg = std::min (maxSeg, (int) ((0.15 + 1.05 * clampd (length, 0, 1)) * fs));
        if (seg == 0) seg = pendingSeg;
    }

    inline void process (double inL, double inR, double& outL, double& outR)
    {
        double hl, hr;
        hall.process (inL, inR, hl, hr);
        ring.push ((hl + hr) * 0.5);
        ringR.push (hl - hr);

        auto read = [this] (int k, double& l, double& s)
        {
            const double w = 0.5 - 0.5 * std::cos (kTwoPi * k / seg);
            l += w * ring.tap (2 * k);
            s += w * ringR.tap (2 * k);
        };
        double m = 0, sd = 0;
        read (j, m, sd);
        read ((j + seg / 2) % seg, m, sd);
        if (++j >= seg) { j = 0; seg = pendingSeg; }   // change length only at a segment boundary
        outL = (m + sd * 0.5) * 1.3;
        outR = (m - sd * 0.5) * 1.3;
    }

private:
    double fs = 48000;
    Fdn hall;
    DelayLine ring, ringR;
    int maxSeg = 48000, seg = 0, pendingSeg = 24000, j = 0;
};
} // namespace dg
