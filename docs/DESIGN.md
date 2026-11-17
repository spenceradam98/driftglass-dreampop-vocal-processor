# Driftglass (working name): design plan

A vocal-chain plugin for dream pop, shoegaze, synth pop and vintage vocal sounds.
JUCE 8 + CMake, VST3 + Standalone, macOS 11+ (universal, Intel safe: scalar C++, no `-march`) and Windows (static CRT).

## Signal flow

```
in -> INPUT -> COLOUR -> DOUBLE -> MOD ----------------------------+--> dry voice
      gain     low/high    doubler    chorus / ensemble /           |
      low cut  cut + reso  octaves    vibrato / flanger / warble    +--> ECHO --+
      comp     crush       fifths                                   |          | (echo also feeds reverb)
      de-ess   wobble      choir                                    +--> REVERB <-+
      drive    noise                                                          |
                                                                    sum (echo + reverb ducked by the dry voice)
                                                                     -> OUTPUT: width, air, mix, gain
```

Each module has its own on/off switch. Echo and reverb are wet returns added to the voice,
and both can duck under the dry vocal so lyrics stay clear while tails bloom between phrases.

## Modules

| Module | Types / controls |
|---|---|
| Input | Gain, Low Cut, one-knob opto-style Compressor (program-dependent release, auto make-up), split-band De-esser, Drive (Tape / Tube / Fuzz) |
| Colour | Low Cut, High Cut, Resonance (telephone / AM radio / old mic band), Crush (bits + sample rate), Wobble (cassette wow/flutter), Noise (hiss + crackle), Mix |
| Double | Double, Octave Up, Octave Down, Fifth Up, Octaves, Choir. Detune, Delay, Width, Mix |
| Mod | Chorus, Ensemble (three-phase string-machine style), Vibrato, Flanger, Warble. Rate, Depth, Mix |
| Echo | Tape (wow, saturation, darker repeats), Analog (BBD-style dark, gritty), Ping-Pong, Diffuse (repeats smear into a cloud). Time or tempo sync, Feedback, Tone, Wow, Drive, Width, Mix, Duck |
| Reverb | Plate (Dattorro-style), Hall (8-line FDN), Cathedral (huge, slow build), Shimmer (octave-up in the feedback loop), Spring (dispersive spring model from Ferro Echo), Reverse (reversed swells), Wash (shoegaze: fuzzed, heavily modulated, very long). Decay, Size, Pre-delay, Tone, Low Cut, Mod, Character (changes meaning per type), Mix, Duck |
| Output | Width, Air, Mix (whole-chain dry/wet), Output gain |

## Presets

About 70 factory presets in six banks: Dream Pop, Shoegaze, Synth Pop, Vintage, Lo-Fi, Space & Utility.
User presets save to the user's application-data folder and show up as a seventh bank.

## Themes

Five themes that change more than colour: Dusk (pastel gradient, soft glow), Neon (synth pop: black, magenta/cyan glow, scanlines),
Fuzz (shoegaze: washed teal, film grain), Velvet (vintage console: cream panels, bakelite knobs), Mono (minimal, high contrast).
Theme and window size are saved with the session.

## Code layout

- `Source/engine`: plain C++17, no JUCE: DSP building blocks, modules, parameter table, presets. Unit tested in `tests/`.
- `Source/plugin`: JUCE processor (APVTS built from the engine's parameter table) and editor.

## Decisions taken for v0.1

- Name: Driftglass (working name, easy to change: `PRODUCT_NAME` in CMakeLists.txt and the title in the editor).
- Double/harmony voices use fixed intervals. A key-aware harmoniser (pitch detection + scale) is planned for v2.
- Mac and Windows builds run in GitHub Actions (`.github/workflows/build.yml`) once the repo is pushed;
  installers come with the other products' packaging later.

## Later

Scale-aware harmoniser (needs pitch detection), oversampled drive, reverb type crossfade (currently a short fade on switch),
installers.
