// Engine tests: parameter table sanity, every preset stable and finite, every reverb and echo
// type produces a tail that decays, silence in gives (near) silence out, and CPU headroom.
#include "Engine.h"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>

using namespace dg;

static int failures = 0;
#define CHECK(cond, ...) do { if (! (cond)) { ++failures; std::printf ("FAIL %s:%d: ", __FILE__, __LINE__); std::printf (__VA_ARGS__); std::printf ("\n"); } } while (0)

struct Result { double peak = 0, rmsBurst = 0, rmsTail = 0, rmsLate = 0; bool finite = true; double seconds = 0; };

// 0.5 s of a vocal-like signal (pulsed, band-limited noise + a harmonic tone), then silence.
static Result run (Engine& e, double fs, double totalSec, bool feed = true)
{
    const int block = 256;
    const int total = (int) (totalSec * fs), burst = (int) (0.5 * fs);
    std::vector<float> inL (block), inR (block), outL (block), outR (block);
    Rng rng (42);
    OnePoleLP lp; lp.set (fs, 3000);
    Result r;
    double sb = 0, st = 0, sl = 0; int nb = 0, nt = 0, nl = 0;
    const auto t0 = std::chrono::steady_clock::now();
    for (int pos = 0; pos < total; pos += block)
    {
        for (int i = 0; i < block; ++i)
        {
            const int k = pos + i;
            double x = 0;
            if (feed && k < burst)
                x = 0.25 * (std::sin (kTwoPi * 220.0 * k / fs) + 0.5 * std::sin (kTwoPi * 440.0 * k / fs)) + 0.15 * lp.process (rng.bi());
            inL[(size_t) i] = inR[(size_t) i] = (float) x;
        }
        e.process (inL.data(), inR.data(), outL.data(), outR.data(), block);
        for (int i = 0; i < block; ++i)
        {
            const int k = pos + i;
            const double a = 0.5 * (outL[(size_t) i] + outR[(size_t) i]);
            if (! std::isfinite (outL[(size_t) i]) || ! std::isfinite (outR[(size_t) i])) r.finite = false;
            r.peak = std::max (r.peak, (double) std::max (std::fabs (outL[(size_t) i]), std::fabs (outR[(size_t) i])));
            if (k < burst) { sb += a * a; ++nb; }
            else if (k < burst + (int) (0.6 * fs)) { st += a * a; ++nt; }
            else if (k >= total - (int) (0.5 * fs)) { sl += a * a; ++nl; }
        }
    }
    r.seconds = std::chrono::duration<double> (std::chrono::steady_clock::now() - t0).count();
    r.rmsBurst = std::sqrt (sb / std::max (1, nb));
    r.rmsTail = std::sqrt (st / std::max (1, nt));
    r.rmsLate = std::sqrt (sl / std::max (1, nl));
    return r;
}

static void applyAll (Engine& e, const std::array<float, NumParams>& v) { for (int i = 0; i < NumParams; ++i) e.setParam (i, v[(size_t) i]); }

int main()
{
    const double fs = 48000;

    // ---- parameter table
    {
        const auto* d = paramDefs();
        for (int i = 0; i < NumParams; ++i)
        {
            CHECK (d[i].min < d[i].max, "%s range", d[i].id);
            CHECK (d[i].def >= d[i].min && d[i].def <= d[i].max, "%s default out of range", d[i].id);
            CHECK (paramIndex (d[i].id) == i, "%s duplicate id", d[i].id);
            if (d[i].kind == Kind::Choice)
            {
                int n = 1; for (const char* c = d[i].choices; *c; ++c) if (*c == '|') ++n;
                CHECK (n == (int) d[i].max + 1, "%s has %d choices but max %g", d[i].id, n, d[i].max);
            }
        }
    }

    // ---- presets: names unique, ids valid, banks valid
    {
        const auto& ps = factoryPresets();
        CHECK (ps.size() >= 60, "only %d presets", (int) ps.size());
        for (size_t i = 0; i < ps.size(); ++i)
        {
            for (const auto& kv : ps[i].values) CHECK (paramIndex (kv.first.c_str()) >= 0, "preset %s: unknown id %s", ps[i].name.c_str(), kv.first.c_str());
            for (size_t j = i + 1; j < ps.size(); ++j) CHECK (ps[i].name != ps[j].name, "duplicate preset name %s", ps[i].name.c_str());
            bool bankOk = false; for (const auto& b : presetBanks()) bankOk |= b == ps[i].bank;
            CHECK (bankOk, "preset %s: unknown bank", ps[i].name.c_str());
        }
        std::printf ("%d presets in %d banks\n", (int) ps.size(), (int) presetBanks().size());
    }

    // ---- every preset: finite, bounded, audible; report CPU
    {
        double worst = 0; std::string worstName;
        for (const auto& p : factoryPresets())
        {
            auto e = std::make_unique<Engine>();
            applyAll (*e, presetValues (p));
            e->prepare (fs, 256);
            const auto r = run (*e, fs, 4.0);
            CHECK (r.finite, "preset %s: non-finite output", p.name.c_str());
            CHECK (r.peak < 3.5, "preset %s: peak %.2f", p.name.c_str(), r.peak);
            CHECK (r.rmsBurst > 0.02, "preset %s: too quiet (%.4f)", p.name.c_str(), r.rmsBurst);
            const double rt = r.seconds / 4.0;
            if (rt > worst) { worst = rt; worstName = p.name; }
            std::printf ("  %-24s %-16s peak %5.2f  burst %6.3f  tail %6.3f  late %7.4f  cpu %4.1f%%\n", p.name.c_str(), p.bank.c_str(),
                         r.peak, r.rmsBurst, r.rmsTail, r.rmsLate, rt * 100.0);
        }
        std::printf ("worst CPU: %s at %.1f%% of one core (unoptimised builds are slower)\n", worstName.c_str(), worst * 100.0);
    }

    // ---- each reverb type: wet tail exists and decays with a short decay
    {
        static const char* names[] = { "Plate", "Hall", "Cathedral", "Shimmer", "Spring", "Reverse", "Wash" };
        for (int t = 0; t < 7; ++t)
        {
            Engine e;
            auto v = presetValues (Preset { "t", "", {} });
            v[EchoOn] = 0; v[VerbType] = (float) t; v[VerbDecay] = 1.5f; v[VerbMix] = 1; v[VerbDuck] = 0; v[VerbPre] = 0;
            applyAll (e, v);
            e.prepare (fs, 256);
            const auto r = run (e, fs, 7.0);
            std::printf ("  reverb %-10s tail %6.4f  late %8.6f  peak %5.2f\n", names[t], r.rmsTail, r.rmsLate, r.peak);
            CHECK (r.finite, "reverb %s non-finite", names[t]);
            CHECK (r.rmsTail > 0.005, "reverb %s: no tail", names[t]);
            CHECK (r.rmsLate < r.rmsTail * 0.05, "reverb %s: does not decay", names[t]);
        }
    }

    // ---- each echo type at max feedback stays bounded
    for (int t = 0; t < 4; ++t)
    {
        Engine e;
        auto v = presetValues (Preset { "t", "", {} });
        v[VerbOn] = 0; v[EchoType] = (float) t; v[EchoFb] = 1; v[EchoMix] = 1; v[EchoDuck] = 0; v[EchoTime] = 150; v[EchoDrive] = 1;
        applyAll (e, v);
        e.prepare (fs, 256);
        const auto r = run (e, fs, 10.0);
        CHECK (r.finite && r.peak < 3.5, "echo type %d unstable (peak %.2f)", t, r.peak);
        std::printf ("  echo type %d at max feedback: peak %.2f late %.3f\n", t, r.peak, r.rmsLate);
    }

    // ---- silence in, silence out (no noise sources on)
    {
        Engine e;
        e.prepare (fs, 256);
        const auto r = run (e, fs, 1.0, false);
        CHECK (r.peak < 1e-6, "silence produced output %.3g", r.peak);
    }

    // ---- other sample rates and a reverb type switch mid-stream
    for (double rate : { 44100.0, 96000.0 })
    {
        Engine e;
        auto v = presetValues (factoryPresets().front());
        applyAll (e, v);
        e.prepare (rate, 256);
        auto r = run (e, rate, 1.0);
        for (int t = 0; t < 7; ++t) { e.setParam (VerbType, (float) t); r = run (e, rate, 0.6); CHECK (r.finite && r.peak < 3.5, "switch to type %d at %g Hz", t, rate); }
    }

    std::printf (failures ? "%d FAILURES\n" : "all tests passed\n", failures);
    return failures ? 1 : 0;
}
