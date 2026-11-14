#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"
#include "Themes.h"

// All controls live in a fixed-size content component that is scaled to the window size.
class DriftglassContent : public juce::Component, private juce::Timer
{
public:
    static constexpr int W = 1040, H = 650;

    explicit DriftglassContent (DriftglassProcessor&);
    ~DriftglassContent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        int param = 0;
        juce::String label;
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };
    struct Module
    {
        juce::String title;
        int onParam = -1, typeParam = -1, type2Param = -1;
        std::vector<Knob*> knobs;
        int cols = 2;
        juce::Rectangle<int> bounds;
        juce::ToggleButton power;
        juce::ComboBox type, type2;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttach;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> typeAttach, type2Attach;
    };

    Module& addModule (const juce::String& title, int onParam, int typeParam, int type2Param, int cols,
                       std::initializer_list<std::pair<int, const char*>> knobs);
    void applyTheme (int index);
    void showPresetMenu();
    void savePresetDialog();
    void timerCallback() override;
    void drawBackground (juce::Graphics&);
    void drawHalo (juce::Graphics&, juce::Rectangle<float>);
    void drawMeters (juce::Graphics&, juce::Rectangle<float>);

    DriftglassProcessor& proc;
    DriftLookAndFeel lnf;
    std::vector<std::unique_ptr<Knob>> knobs;
    std::vector<std::unique_ptr<Module>> modules;
    Module* outputModule = nullptr;
    Knob* charKnob = nullptr;
    Knob* timeKnob = nullptr;

    juce::TextButton presetButton, prevButton { "<" }, nextButton { ">" }, saveButton { "SAVE" };
    juce::ComboBox themeBox;
    juce::Image background;
    juce::Rectangle<float> haloArea, meterArea;
    float halo = 0, inMeter = 0, outMeter = 0, grMeter = 0, phase = 0;
    juce::String shownPreset;
    std::unique_ptr<juce::AlertWindow> saveWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DriftglassContent)
};

class DriftglassEditor : public juce::AudioProcessorEditor
{
public:
    explicit DriftglassEditor (DriftglassProcessor&);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (juce::Colours::black); }

private:
    DriftglassProcessor& proc;
    DriftglassContent content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DriftglassEditor)
};
