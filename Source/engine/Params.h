// The parameter table: the single source of truth for the engine, the plugin's
// AudioProcessorValueTreeState, the presets and the editor.
#pragma once

namespace dg
{
enum class Kind { Float, Choice, Bool };

struct ParamDef
{
    const char* id;
    const char* name;
    Kind kind;
    float min, max, def;
    float centre;          // value shown at 12 o'clock for skewed ranges (0 = linear)
    const char* unit;
    const char* choices;   // '|' separated, for Choice
};

// X(enumName, id, display name, kind, min, max, default, centre, unit, choices)
#define DG_PARAMS(X) \
    X (InOn,       "in_on",       "Input On",          Bool,   0, 1, 1, 0, "", "") \
    X (InGain,     "in_gain",     "Input Gain",        Float, -24, 24, 0, 0, "dB", "") \
    X (InLowCut,   "in_lowcut",   "Input Low Cut",     Float, 20, 400, 80, 90, "Hz", "") \
    X (Comp,       "comp",        "Compress",          Float,  0, 1, 0.3f, 0, "", "") \
    X (DeEss,      "deess",       "De-ess",            Float,  0, 1, 0.3f, 0, "", "") \
    X (Drive,      "drive",       "Drive",             Float,  0, 1, 0.12f, 0, "", "") \
    X (DriveType,  "drive_type",  "Drive Type",        Choice, 0, 2, 0, 0, "", "Tape|Tube|Fuzz") \
    X (ColOn,      "col_on",      "Colour On",         Bool,   0, 1, 0, 0, "", "") \
    X (ColLowCut,  "col_lowcut",  "Colour Low Cut",    Float, 20, 2000, 20, 200, "Hz", "") \
    X (ColHighCut, "col_highcut", "Colour High Cut",   Float, 1000, 20000, 20000, 5000, "Hz", "") \
    X (ColReso,    "col_reso",    "Resonance",         Float,  0, 1, 0, 0, "", "") \
    X (ColCrush,   "col_crush",   "Crush",             Float,  0, 1, 0, 0, "", "") \
    X (ColWobble,  "col_wobble",  "Wobble",            Float,  0, 1, 0, 0, "", "") \
    X (ColNoise,   "col_noise",   "Noise",             Float,  0, 1, 0, 0, "", "") \
    X (ColMix,     "col_mix",     "Colour Mix",        Float,  0, 1, 1, 0, "", "") \
    X (DblOn,      "dbl_on",      "Double On",         Bool,   0, 1, 0, 0, "", "") \
    X (DblMode,    "dbl_mode",    "Double Mode",       Choice, 0, 5, 0, 0, "", "Double|Octave Up|Octave Down|Fifth Up|Octaves|Choir") \
    X (DblDetune,  "dbl_detune",  "Detune",            Float,  0, 1, 0.4f, 0, "", "") \
    X (DblDelay,   "dbl_delay",   "Double Delay",      Float,  0, 1, 0.4f, 0, "", "") \
    X (DblWidth,   "dbl_width",   "Double Width",      Float,  0, 1, 0.8f, 0, "", "") \
    X (DblMix,     "dbl_mix",     "Double Mix",        Float,  0, 1, 0.5f, 0, "", "") \
    X (ModOn,      "mod_on",      "Mod On",            Bool,   0, 1, 0, 0, "", "") \
    X (ModMode,    "mod_mode",    "Mod Mode",          Choice, 0, 4, 0, 0, "", "Chorus|Ensemble|Vibrato|Flanger|Warble") \
    X (ModRate,    "mod_rate",    "Mod Rate",          Float, 0.05f, 8, 0.8f, 1.0f, "Hz", "") \
    X (ModDepth,   "mod_depth",   "Mod Depth",         Float,  0, 1, 0.5f, 0, "", "") \
    X (ModMix,     "mod_mix",     "Mod Mix",           Float,  0, 1, 0.5f, 0, "", "") \
    X (EchoOn,     "echo_on",     "Echo On",           Bool,   0, 1, 1, 0, "", "") \
    X (EchoType,   "echo_type",   "Echo Type",         Choice, 0, 3, 0, 0, "", "Tape|Analog|Ping-Pong|Diffuse") \
    X (EchoTime,   "echo_time",   "Echo Time",         Float, 10, 2000, 380, 300, "ms", "") \
    X (EchoSync,   "echo_sync",   "Echo Sync",         Choice, 0, 12, 0, 0, "", "Free|1/32|1/16T|1/16|1/16D|1/8T|1/8|1/8D|1/4T|1/4|1/4D|1/2|1/1") \
    X (EchoFb,     "echo_fb",     "Feedback",          Float,  0, 1, 0.35f, 0, "", "") \
    X (EchoTone,   "echo_tone",   "Echo Tone",         Float,  0, 1, 0.55f, 0, "", "") \
    X (EchoWow,    "echo_wow",    "Echo Wow",          Float,  0, 1, 0.2f, 0, "", "") \
    X (EchoDrive,  "echo_drive",  "Echo Drive",        Float,  0, 1, 0.2f, 0, "", "") \
    X (EchoWidth,  "echo_width",  "Echo Width",        Float,  0, 1, 0.5f, 0, "", "") \
    X (EchoMix,    "echo_mix",    "Echo Mix",          Float,  0, 1, 0.22f, 0, "", "") \
    X (EchoDuck,   "echo_duck",   "Echo Duck",         Float,  0, 1, 0.3f, 0, "", "") \
    X (VerbOn,     "verb_on",     "Reverb On",         Bool,   0, 1, 1, 0, "", "") \
    X (VerbType,   "verb_type",   "Reverb Type",       Choice, 0, 6, 0, 0, "", "Plate|Hall|Cathedral|Shimmer|Spring|Reverse|Wash") \
    X (VerbDecay,  "verb_decay",  "Decay",             Float, 0.2f, 30, 2.5f, 3.0f, "s", "") \
    X (VerbSize,   "verb_size",   "Size",              Float,  0, 1, 0.6f, 0, "", "") \
    X (VerbPre,    "verb_pre",    "Pre-delay",         Float,  0, 250, 20, 40, "ms", "") \
    X (VerbTone,   "verb_tone",   "Reverb Tone",       Float,  0, 1, 0.55f, 0, "", "") \
    X (VerbLowCut, "verb_lowcut", "Reverb Low Cut",    Float, 20, 1000, 150, 150, "Hz", "") \
    X (VerbMod,    "verb_mod",    "Reverb Mod",        Float,  0, 1, 0.3f, 0, "", "") \
    X (VerbChar,   "verb_char",   "Character",         Float,  0, 1, 0.5f, 0, "", "") \
    X (VerbMix,    "verb_mix",    "Reverb Mix",        Float,  0, 1, 0.28f, 0, "", "") \
    X (VerbDuck,   "verb_duck",   "Reverb Duck",       Float,  0, 1, 0.15f, 0, "", "") \
    X (OutWidth,   "out_width",   "Width",             Float,  0, 2, 1, 0, "", "") \
    X (OutAir,     "out_air",     "Air",               Float,  0, 1, 0.2f, 0, "", "") \
    X (OutMix,     "out_mix",     "Mix",               Float,  0, 1, 1, 0, "", "") \
    X (OutGain,    "out_gain",    "Output",            Float, -24, 12, 0, 0, "dB", "")

enum P : int
{
#define DG_ENUM(e, ...) e,
    DG_PARAMS (DG_ENUM)
#undef DG_ENUM
    NumParams
};

inline const ParamDef* paramDefs()
{
#define DG_DEF(e, id, name, kind, mn, mx, df, ct, unit, ch) ParamDef { id, name, Kind::kind, (float) (mn), (float) (mx), (float) (df), (float) (ct), unit, ch },
    static const ParamDef defs[NumParams] = { DG_PARAMS (DG_DEF) };
#undef DG_DEF
    return defs;
}

// Index for an id string, or -1.
int paramIndex (const char* id);

// Labels for the reverb "Character" knob, which means something different per type.
inline const char* characterLabel (int verbType)
{
    static const char* labels[] = { "Diffusion", "Diffusion", "Build", "Shimmer", "Drip", "Length", "Fuzz" };
    return (verbType >= 0 && verbType < 7) ? labels[verbType] : "Character";
}
} // namespace dg
