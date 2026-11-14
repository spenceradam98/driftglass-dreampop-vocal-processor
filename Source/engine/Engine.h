// Driftglass engine: the whole vocal chain. Plain C++17, no JUCE, so it can be unit tested.
#pragma once

#include "Modules.h"
#include "Params.h"
#include "Reverbs.h"
#include <atomic>
#include <string>
#include <utility>
#include <vector>

namespace dg
{
class Engine
{
public:
    Engine();

    void prepare (double sampleRate, int maxBlock);
    void reset();

    // Parameter values in their natural units (as in the ParamDef table).
    void setParam (int index, float value) { if (index >= 0 && index < NumParams) values[(size_t) index] = value; }
    float getParam (int index) const { return values[(size_t) index]; }
    void setTempo (double bpm) { if (bpm > 20 && bpm < 400) tempo = bpm; }

    void process (const float* inL, const float* inR, float* outL, float* outR, int numSamples);

    // Metering for the UI (written on the audio thread).
    std::atomic<float> inLevel { 0 }, outLevel { 0 }, compGr { 0 }, deessGr { 0 }, verbLevel { 0 };

    double echoTimeMs() const;   // resolved echo time (free or tempo synced)

private:
    void update();
    void configureReverb (int type);

    double fs = 48000, tempo = 120;
    std::array<float, NumParams> values {};

    InputStage input;
    ColourStage colour;
    Doubler doubler;
    ModStage mod;
    Echo echo;
    Fdn fdn;
    Plate plate;
    SpringTank springL, springR;
    ReverseVerb reverse;

    DelayLine preL, preR;
    Biquad verbHp[2], airShelf[2];
    EnvFollower duckEnv;
    Smoothed onIn, onCol, onDbl, onMod, onEcho, onVerb, echoMix, verbMix, outMix, outGain, width, verbFade, duckE, duckV;
    double preSamples = 0;
    int verbType = -1;
    float meterIn = 0, meterOut = 0, meterVerb = 0;
};

// ---------------------------------------------------------------- Presets
struct Preset
{
    std::string name;
    std::string bank;
    std::vector<std::pair<std::string, float>> values;   // overrides on top of the parameter defaults
};

const std::vector<Preset>& factoryPresets();
const std::vector<std::string>& presetBanks();

// Full parameter vector for a preset (defaults + overrides).
std::array<float, NumParams> presetValues (const Preset&);
} // namespace dg
