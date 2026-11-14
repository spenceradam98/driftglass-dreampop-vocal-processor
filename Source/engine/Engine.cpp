#include "Engine.h"
#include <cstring>

namespace dg
{
int paramIndex (const char* id)
{
    const auto* d = paramDefs();
    for (int i = 0; i < NumParams; ++i)
        if (std::strcmp (d[i].id, id) == 0) return i;
    return -1;
}

Engine::Engine()
{
    const auto* d = paramDefs();
    for (int i = 0; i < NumParams; ++i) values[(size_t) i] = d[i].def;
}

void Engine::prepare (double sampleRate, int)
{
    fs = sampleRate;
    input.prepare (fs);
    colour.prepare (fs);
    doubler.prepare (fs);
    mod.prepare (fs);
    echo.prepare (fs);
    fdn.prepare (fs);
    plate.prepare (fs);
    springL.prepare (fs, 0);
    springR.prepare (fs, 1);
    reverse.prepare (fs);
    preL.allocate ((int) (0.32 * fs));
    preR.allocate ((int) (0.32 * fs));
    duckEnv.set (fs, 5, 280);

    const double t = 0.02;
    for (auto* s : { &onIn, &onCol, &onDbl, &onMod, &onEcho, &onVerb, &echoMix, &verbMix, &outMix, &outGain, &width, &duckE, &duckV })
        s->prepare (fs, t, 0);
    verbFade.prepare (fs, 0.04, 1);
    verbType = -1;
    update();
    for (auto* s : { &onIn, &onCol, &onDbl, &onMod, &onEcho, &onVerb, &echoMix, &verbMix, &outMix, &outGain, &width })
        s->snap();
    duckE.v = duckV.v = 1;
    reset();
}

void Engine::reset()
{
    input.reset(); colour.reset(); doubler.reset(); mod.reset(); echo.reset();
    fdn.clear(); plate.clear(); springL.clear(); springR.clear(); reverse.clear();
    preL.clear(); preR.clear();
    for (int c = 0; c < 2; ++c) { verbHp[c].reset(); airShelf[c].reset(); }
    duckEnv.reset();
}

double Engine::echoTimeMs() const
{
    static const double beats[] = { 0, 0.125, 1.0 / 6.0, 0.25, 0.375, 1.0 / 3.0, 0.5, 0.75, 2.0 / 3.0, 1.0, 1.5, 2.0, 4.0 };
    const int sync = (int) values[EchoSync];
    if (sync <= 0 || sync > 12) return values[EchoTime];
    return clampd (beats[sync] * 60000.0 / tempo, 10.0, 2000.0);
}

void Engine::configureReverb (int type)
{
    const double decay = values[VerbDecay], size = values[VerbSize], mods = values[VerbMod], ch = values[VerbChar];
    const double dampHz = 1500.0 * std::pow (2.0, values[VerbTone] * 3.7);
    Fdn::Config c;
    c.rt60 = decay; c.dampHz = dampHz; c.modDepth = mods;
    switch (type)
    {
        case 0: plate.configure (decay, size, dampHz, mods, ch); break;
        case 1: c.size = 0.6 + 1.0 * size; c.diffusion = ch; c.modRate = 0.5; fdn.configure (c); break;
        case 2: c.size = 1.4 + 1.5 * size; c.diffusion = 0.5 + 0.5 * ch; c.modRate = 0.3; c.dampHz = dampHz * 0.8; fdn.configure (c); break;
        case 3: c.size = 0.8 + 1.0 * size; c.diffusion = 0.7; c.shimmer = ch; c.modRate = 0.45; fdn.configure (c); break;
        case 4:
        {
            const double rt = clampd (decay, 0.4, 8.0), tone = 2000.0 + values[VerbTone] * 5000.0;
            springL.configure (rt, ch, tone); springR.configure (rt, ch, tone);
            break;
        }
        case 5: reverse.configure (decay, ch, dampHz, mods); break;
        default:
            c.size = 1.0 + 1.6 * size; c.diffusion = 0.9; c.fuzz = ch; c.randomMod = true;
            c.modDepth = 0.4 + mods * 0.6; c.modRate = 0.25; fdn.configure (c);
            break;
    }
}

void Engine::update()
{
    const auto& v = values;
    input.configure (v[InGain], v[InLowCut], v[Comp], v[DeEss], v[Drive], (int) v[DriveType]);
    colour.configure (v[ColLowCut], v[ColHighCut], v[ColReso], v[ColCrush], v[ColWobble], v[ColNoise], v[ColMix]);
    doubler.configure ((int) v[DblMode], v[DblDetune], v[DblDelay], v[DblWidth], v[DblMix]);
    mod.configure ((int) v[ModMode], v[ModRate], v[ModDepth], v[ModMix]);
    echo.configure ((int) v[EchoType], echoTimeMs(), v[EchoFb], v[EchoTone], v[EchoWow], v[EchoDrive], v[EchoWidth]);

    const int type = (int) v[VerbType];
    if (type != verbType)
    {
        // switch: clear the incoming algorithm and fade it in
        if (type == 0) plate.clear();
        else if (type == 4) { springL.clear(); springR.clear(); }
        else if (type == 5) reverse.clear();
        else fdn.clear();
        verbType = type;
        verbFade.v = 0;
    }
    verbFade.set (1);
    configureReverb (type);

    preSamples = (v[VerbPre] + (type == 2 ? 30.0 : 0.0)) * 0.001 * fs;
    for (int c = 0; c < 2; ++c)
    {
        verbHp[c].highpass (fs, v[VerbLowCut], 0.6);
        airShelf[c].highShelf (fs, 10000, v[OutAir] * 9.0, 0.7);
    }

    onIn.set (v[InOn] > 0.5f ? 1 : 0);
    onCol.set (v[ColOn] > 0.5f ? 1 : 0);
    onDbl.set (v[DblOn] > 0.5f ? 1 : 0);
    onMod.set (v[ModOn] > 0.5f ? 1 : 0);
    onEcho.set (v[EchoOn] > 0.5f ? 1 : 0);
    onVerb.set (v[VerbOn] > 0.5f ? 1 : 0);
    echoMix.set (v[EchoMix]);
    verbMix.set (v[VerbMix]);
    outMix.set (v[OutMix]);
    outGain.set (dbToGain (v[OutGain]));
    width.set (v[OutWidth]);
}

void Engine::process (const float* inL, const float* inR, float* outL, float* outR, int n)
{
    update();
    const double duckEAmt = values[EchoDuck] * 14.0, duckVAmt = values[VerbDuck] * 14.0;
    float pkIn = 0, pkOut = 0, pkVerb = 0;

    for (int i = 0; i < n; ++i)
    {
        const double dryL = inL[i], dryR = inR[i];
        pkIn = std::max (pkIn, (float) std::max (std::fabs (dryL), std::fabs (dryR)));
        double l = dryL, r = dryR;

        // series modules: each crossfades between its input and its output when switched
        auto stage = [&] (Smoothed& on, auto&& fn)
        {
            const double g = on.next();
            if (g < 1e-4 && on.target == 0) return;
            double pl = l, pr = r;
            fn (pl, pr);
            l += (pl - l) * g; r += (pr - r) * g;
        };
        stage (onIn, [this] (double& a, double& b) { input.process (a, b); });
        stage (onCol, [this] (double& a, double& b) { colour.process (a, b); });
        stage (onDbl, [this] (double& a, double& b) { doubler.process (a, b); });
        stage (onMod, [this] (double& a, double& b) { mod.process (a, b); });

        // ducking follows the processed voice
        const double env = duckEnv.process (0.5 * (l + r));
        duckE.set (1.0 / (1.0 + duckEAmt * env));
        duckV.set (1.0 / (1.0 + duckVAmt * env));
        const double dE = duckE.next(), dV = duckV.next();

        // echo
        double eL = 0, eR = 0;
        const double gE = onEcho.next();
        if (gE > 1e-4 || onEcho.target > 0)
        {
            echo.process (l * gE, r * gE, eL, eR);
            const double m = echoMix.next() * dE;
            eL *= m; eR *= m;
        }
        else echoMix.next();

        // reverb (fed by the voice plus the echoes)
        double vL = 0, vR = 0;
        const double gV = onVerb.next();
        if (gV > 1e-4 || onVerb.target > 0)
        {
            preL.push (verbHp[0].process ((l + eL * 0.6) * gV));
            preR.push (verbHp[1].process ((r + eR * 0.6) * gV));
            const double xl = preSamples < 1 ? preL.tap (0) : preL.tapLin (preSamples);
            const double xr = preSamples < 1 ? preR.tap (0) : preR.tapLin (preSamples);
            switch (verbType)
            {
                case 0: plate.process (xl, xr, vL, vR); break;
                case 4: vL = springL.process (0.5 * (xl + xr)); vR = springR.process (0.5 * (xl + xr)); break;
                case 5: reverse.process (xl, xr, vL, vR); break;
                default: fdn.process (xl, xr, vL, vR); break;
            }
            const double m = verbMix.next() * dV * verbFade.next();
            vL *= m; vR *= m;
            pkVerb = std::max (pkVerb, (float) std::fabs (vL + vR) * 0.5f);
        }
        else verbMix.next();

        double wl = l + eL + vL, wr = r + eR + vR;

        // output: width, air, dry/wet, gain
        const double w = width.next();
        const double mid = 0.5 * (wl + wr), side = 0.5 * (wl - wr) * w;
        wl = airShelf[0].process (mid + side);
        wr = airShelf[1].process (mid - side);
        const double mx = outMix.next(), og = outGain.next();
        double ol = (dryL + (wl - dryL) * mx) * og;
        double orr = (dryR + (wr - dryR) * mx) * og;

        if (isBad (ol) || isBad (orr)) { reset(); ol = orr = 0; }   // never let a blow-up reach the host
        ol = clampd (ol, -4.0, 4.0); orr = clampd (orr, -4.0, 4.0);
        outL[i] = (float) ol; outR[i] = (float) orr;
        pkOut = std::max (pkOut, (float) std::max (std::fabs (ol), std::fabs (orr)));
    }

    // meter ballistics (block rate)
    const float fall = (float) std::exp (-(double) n / (0.25 * fs));
    meterIn = std::max (pkIn, meterIn * fall);
    meterOut = std::max (pkOut, meterOut * fall);
    meterVerb = std::max (pkVerb, meterVerb * fall);
    inLevel.store (meterIn); outLevel.store (meterOut); verbLevel.store (meterVerb);
    compGr.store ((float) input.compGr); deessGr.store ((float) input.deessGr);
}
} // namespace dg
