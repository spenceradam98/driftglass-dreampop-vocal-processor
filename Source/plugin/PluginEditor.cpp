#include "PluginEditor.h"

using namespace juce;

namespace
{
constexpr int kMargin = 14, kHeaderH = 70, kGap = 12, kRowH = 272, kCell = 78, kMeterW = 70;
constexpr float kStart = MathConstants<float>::pi * -0.78f, kEnd = MathConstants<float>::pi * 0.78f;

Font titleFont (float size) { return Font (FontOptions (size, Font::bold)).withExtraKerningFactor (0.18f); }
}

// ============================================================================ content
DriftglassContent::DriftglassContent (DriftglassProcessor& p) : proc (p)
{
    setLookAndFeel (&lnf);
    using namespace dg;

    addModule ("Input", InOn, DriveType, -1, 3, { { InGain, "Gain" }, { InLowCut, "Low Cut" }, { Comp, "Compress" }, { DeEss, "De-ess" }, { Drive, "Drive" } });
    addModule ("Colour", ColOn, -1, -1, 4, { { ColLowCut, "Low Cut" }, { ColHighCut, "High Cut" }, { ColReso, "Resonance" }, { ColCrush, "Crush" },
                                             { ColWobble, "Wobble" }, { ColNoise, "Noise" }, { ColMix, "Mix" } });
    addModule ("Double", DblOn, DblMode, -1, 2, { { DblDetune, "Detune" }, { DblDelay, "Delay" }, { DblWidth, "Width" }, { DblMix, "Mix" } });
    addModule ("Mod", ModOn, ModMode, -1, 2, { { ModRate, "Rate" }, { ModDepth, "Depth" }, { ModMix, "Mix" } });
    addModule ("Echo", EchoOn, EchoType, EchoSync, 4, { { EchoTime, "Time" }, { EchoFb, "Feedback" }, { EchoTone, "Tone" }, { EchoWow, "Wow" },
                                                        { EchoDrive, "Drive" }, { EchoWidth, "Width" }, { EchoMix, "Mix" }, { EchoDuck, "Duck" } });
    addModule ("Reverb", VerbOn, VerbType, -1, 5, { { VerbDecay, "Decay" }, { VerbSize, "Size" }, { VerbPre, "Pre-delay" }, { VerbTone, "Tone" },
                                                     { VerbLowCut, "Low Cut" }, { VerbMod, "Mod" }, { VerbChar, "Character" }, { VerbMix, "Mix" }, { VerbDuck, "Duck" } });
    outputModule = &addModule ("Output", -1, -1, -1, 2, { { OutWidth, "Width" }, { OutAir, "Air" }, { OutMix, "Mix" }, { OutGain, "Output" } });

    for (auto& k : knobs)
    {
        if (k->param == VerbChar) charKnob = k.get();
        if (k->param == EchoTime) timeKnob = k.get();
        if (k->param == InGain || k->param == OutGain) k->slider.getProperties().set ("bipolar", true);
    }

    presetButton.onClick = [this] { showPresetMenu(); };
    prevButton.onClick = [this] { proc.stepPreset (-1); };
    nextButton.onClick = [this] { proc.stepPreset (1); };
    saveButton.onClick = [this] { savePresetDialog(); };
    presetButton.setTooltip ("Browse presets");
    saveButton.setTooltip ("Save the current settings as a user preset");
    for (auto* b : { &presetButton, &prevButton, &nextButton, &saveButton }) addAndMakeVisible (*b);

    for (int i = 0; i < (int) themes().size(); ++i) themeBox.addItem (themes()[(size_t) i].name, i + 1);
    themeBox.onChange = [this] { applyTheme (themeBox.getSelectedId() - 1); };
    addAndMakeVisible (themeBox);

    setSize (W, H);
    applyTheme (jlimit (0, (int) themes().size() - 1, proc.themeIndex));
    themeBox.setSelectedId (proc.themeIndex + 1, dontSendNotification);
    startTimerHz (30);
}

DriftglassContent::~DriftglassContent()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

DriftglassContent::Module& DriftglassContent::addModule (const String& title, int onParam, int typeParam, int type2Param, int cols,
                                                         std::initializer_list<std::pair<int, const char*>> list)
{
    auto m = std::make_unique<Module>();
    m->title = title; m->onParam = onParam; m->typeParam = typeParam; m->type2Param = type2Param; m->cols = cols;
    const auto* defs = dg::paramDefs();

    if (onParam >= 0)
    {
        m->power.setTooltip ("Switch " + title + " on or off");
        addAndMakeVisible (m->power);
        m->powerAttach = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, defs[onParam].id, m->power);
    }
    auto setupBox = [&] (ComboBox& box, int param, std::unique_ptr<AudioProcessorValueTreeState::ComboBoxAttachment>& att)
    {
        if (param < 0) return;
        box.addItemList (StringArray::fromTokens (defs[param].choices, "|", ""), 1);
        box.setJustificationType (Justification::centred);
        addAndMakeVisible (box);
        att = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, defs[param].id, box);
    };
    setupBox (m->type, typeParam, m->typeAttach);
    setupBox (m->type2, type2Param, m->type2Attach);

    for (const auto& kv : list)
    {
        auto k = std::make_unique<Knob>();
        k->param = kv.first;
        k->label = kv.second;
        k->slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        k->slider.setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        k->slider.setRotaryParameters (MathConstants<float>::twoPi + kStart, MathConstants<float>::twoPi + kEnd, true);
        k->slider.setPopupDisplayEnabled (true, true, this, 1200);
        k->slider.setTooltip (defs[kv.first].name);
        addAndMakeVisible (k->slider);
        k->attach = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, defs[kv.first].id, k->slider);
        if (auto* prm = proc.apvts.getParameter (defs[kv.first].id))
            k->slider.setDoubleClickReturnValue (true, prm->convertFrom0to1 (prm->getDefaultValue()));
        m->knobs.push_back (k.get());
        knobs.push_back (std::move (k));
    }
    modules.push_back (std::move (m));
    return *modules.back();
}

void DriftglassContent::applyTheme (int index)
{
    index = jlimit (0, (int) themes().size() - 1, index);
    proc.themeIndex = index;
    const auto& t = themes()[(size_t) index];
    lnf.setTheme (t);

    // pre-render the background at 2x: gradient plus the theme's texture
    background = Image (Image::ARGB, W * 2, H * 2, true);
    Graphics g (background);
    g.addTransform (AffineTransform::scale (2.0f));
    g.setGradientFill (ColourGradient (t.bgTop, 0, 0, t.bgBottom, (float) W * 0.3f, (float) H, false));
    g.fillAll();
    Random rnd (7);
    if (t.stars)
        for (int i = 0; i < 140; ++i)
        {
            const float x = rnd.nextFloat() * W, y = rnd.nextFloat() * H * 0.7f, s = 0.6f + rnd.nextFloat() * 1.6f;
            g.setColour (Colours::white.withAlpha (0.08f + 0.35f * rnd.nextFloat() * (1.0f - y / H)));
            g.fillEllipse (x, y, s, s);
        }
    if (t.glow)
    {
        g.setGradientFill (ColourGradient (t.accent.withAlpha (0.22f), W * 0.82f, H * 0.05f, t.accent.withAlpha (0.0f), W * 0.82f + 420, H * 0.05f + 420, true));
        g.fillAll();
    }
    if (t.grain)
        for (int i = 0; i < W * H / 6; ++i)
        {
            g.setColour ((rnd.nextBool() ? Colours::white : Colours::black).withAlpha (t.grainAmount * rnd.nextFloat()));
            g.fillRect (rnd.nextFloat() * W, rnd.nextFloat() * H, 1.0f, 1.0f);
        }
    if (t.scanlines)
    {
        g.setColour (Colours::black.withAlpha (0.22f));
        for (int y = 0; y < H; y += 3) g.fillRect (0.0f, (float) y, (float) W, 1.0f);
    }

    for (auto& m : modules) { m->type.setColour (ComboBox::textColourId, t.text); m->type2.setColour (ComboBox::textColourId, t.text); }
    sendLookAndFeelChange();
    repaint();
}

// ---------------------------------------------------------------- layout
void DriftglassContent::resized()
{
    // header
    const int hy = 18;
    prevButton.setBounds (352, hy, 30, 32);
    presetButton.setBounds (386, hy, 260, 32);
    nextButton.setBounds (650, hy, 30, 32);
    saveButton.setBounds (688, hy, 58, 32);
    themeBox.setBounds (762, hy + 2, 110, 28);
    haloArea = Rectangle<float> (886, 6, (float) (W - 886 - kMargin), 58);

    // two rows of modules, widths scaled to fill the row
    auto layoutRow = [this] (int first, int last, int y)
    {
        int sum = 0;
        auto natural = [this] (const Module& m) { return m.cols * kCell + 20 + (&m == outputModule ? kMeterW : 0); };
        for (int i = first; i <= last; ++i) sum += natural (*modules[(size_t) i]);
        const float scale = (float) (W - 2 * kMargin - (last - first) * kGap) / (float) sum;
        float x = (float) kMargin;
        for (int i = first; i <= last; ++i)
        {
            auto& m = *modules[(size_t) i];
            const int w = (i == last) ? (W - kMargin - (int) x) : (int) std::round (natural (m) * scale);
            m.bounds = { (int) x, y, w, kRowH };
            x += (float) (w + kGap);

            auto head = m.bounds.withHeight (38).reduced (10, 8);
            auto body = m.bounds.withTrimmedTop (40).reduced (6, 4);
            if (m.onParam >= 0) m.power.setBounds (head.removeFromLeft (36).withSizeKeepingCentre (36, 18));
            if (m.type2Param >= 0) { m.type2.setBounds (head.removeFromRight (64)); head.removeFromRight (6); }
            if (m.typeParam >= 0)
            {
                if (w < 230)   // narrow panel: the type selector gets its own line under the title
                    m.type.setBounds (body.removeFromTop (30).reduced (6, 3));
                else
                    m.type.setBounds (head.removeFromRight (m.type2Param >= 0 ? 86 : 104));
            }
            if (&m == outputModule) meterArea = body.removeFromRight (kMeterW).reduced (6, 8).toFloat();
            const int rows = ((int) m.knobs.size() + m.cols - 1) / m.cols;
            const int cw = body.getWidth() / m.cols, ch = body.getHeight() / jmax (1, rows);
            const int knobSize = jmin (72, cw - 4, ch - 28);
            for (size_t k = 0; k < m.knobs.size(); ++k)
            {
                const int c = (int) k % m.cols, r = (int) k / m.cols;
                auto cell = Rectangle<int> (body.getX() + c * cw, body.getY() + r * ch, cw, ch);
                m.knobs[k]->slider.setBounds (cell.withTrimmedBottom (26).withSizeKeepingCentre (knobSize, knobSize));
            }
        }
    };
    layoutRow (0, 3, kHeaderH + 10);
    layoutRow (4, 6, kHeaderH + 10 + kRowH + kGap);
}

// ---------------------------------------------------------------- painting
void DriftglassContent::paint (Graphics& g)
{
    const auto& t = lnf.theme();
    g.drawImage (background, getLocalBounds().toFloat());

    // title (on light-panel themes the header sits on the dark case, so use the panel colour)
    const bool lightPanel = (t.panel.getPerceivedBrightness() > 0.6f && t.panel.getFloatAlpha() > 0.8f);
    const auto headText = lightPanel ? t.panel : t.text, headDim = lightPanel ? t.panel.withAlpha (0.65f) : t.textDim;
    g.setColour (headText);
    g.setFont (titleFont (30.0f));
    g.drawText ("driftglass", 22, 10, 300, 36, Justification::centredLeft);
    g.setColour (headDim);
    g.setFont (titleFont (11.0f));
    g.drawText ("DREAM VOCAL PROCESSOR", 24, 44, 300, 16, Justification::centredLeft);

    drawHalo (g, haloArea);

    for (auto& mp : modules)
    {
        auto& m = *mp;
        const auto r = m.bounds.toFloat();
        const bool on = m.onParam < 0 || m.power.getToggleState();
        if (t.glow && on)
        {
            g.setColour (t.accent.withAlpha (0.07f));
            g.fillRoundedRectangle (r.expanded (3), t.corner + 3);
        }
        g.setColour (t.panel);
        g.fillRoundedRectangle (r, t.corner);
        g.setColour (on ? t.panelEdge : t.panelEdge.withMultipliedAlpha (0.5f));
        g.drawRoundedRectangle (r.reduced (0.5f), t.corner, 1.0f);
        if (t.knobStyle == 3)   // console screws
            for (auto pt : { r.getTopLeft().translated (8, 8), r.getTopRight().translated (-8, 8), r.getBottomLeft().translated (8, -8), r.getBottomRight().translated (-8, -8) })
            {
                g.setColour (Colour (0xffa08a62)); g.fillEllipse (Rectangle<float> (7, 7).withCentre (pt));
                g.setColour (Colour (0xff5e4c32)); g.drawLine (pt.x - 2.5f, pt.y - 2.5f, pt.x + 2.5f, pt.y + 2.5f, 1.0f);
            }

        // module title
        auto head = m.bounds.withHeight (38).reduced (12, 8);
        if (m.onParam >= 0) head.removeFromLeft (40);
        g.setColour (on ? (t.knobStyle == 1 ? t.accent2 : t.text) : t.textDim);
        g.setFont (titleFont (13.0f));
        g.drawText (m.title.toUpperCase(), head, Justification::centredLeft);
        g.setColour (t.panelEdge.withMultipliedAlpha (0.6f));
        g.drawHorizontalLine (m.bounds.getY() + 38, r.getX() + 10, r.getRight() - 10);

        // knob labels
        for (auto* k : m.knobs)
        {
            const auto sb = k->slider.getBounds();
            const bool en = k->slider.isEnabled();
            g.setColour ((on && en) ? t.text.withAlpha (0.9f) : t.textDim.withAlpha (0.6f));
            g.setFont (Font (FontOptions (12.0f, Font::bold)));
            g.drawText (k->label, sb.getX() - 12, sb.getBottom() + 1, sb.getWidth() + 24, 14, Justification::centred);
            g.setColour (t.textDim.withAlpha (on ? 0.9f : 0.5f));
            g.setFont (Font (FontOptions (11.0f)));
            String txt = k->slider.getTextFromValue (k->slider.getValue());
            if (! en) txt = "synced";
            g.drawText (txt, sb.getX() - 12, sb.getBottom() + 14, sb.getWidth() + 24, 13, Justification::centred);
        }
    }
    drawMeters (g, meterArea);
}

void DriftglassContent::drawHalo (Graphics& g, Rectangle<float> area)
{
    // a soft, breathing halo that follows the output and the reverb level
    const auto& t = lnf.theme();
    const auto c = Point<float> (area.getRight() - 34, area.getCentreY());
    const float lvl = jlimit (0.0f, 1.0f, std::sqrt (outMeter) * 1.2f);
    for (int i = 5; i >= 0; --i)
    {
        const float rad = 8.0f + i * 4.0f + lvl * 6.0f + std::sin (phase + i * 0.7f) * 1.5f;
        const auto col = t.accent.interpolatedWith (t.accent2, i / 5.0f);
        g.setColour (col.withAlpha ((0.10f + 0.5f * halo) * (1.0f - i / 6.0f) + 0.05f));
        g.drawEllipse (Rectangle<float> (rad * 2, rad * 2).withCentre (c), 1.6f);
    }
    g.setColour (t.text.withAlpha (0.6f + 0.4f * lvl));
    g.fillEllipse (Rectangle<float> (6, 6).withCentre (c));
    g.setColour ((t.panel.getPerceivedBrightness() > 0.6f && t.panel.getFloatAlpha() > 0.8f) ? t.panel.withAlpha (0.65f) : t.textDim);
    g.setFont (titleFont (10.0f));
    g.drawText ("HALO", area.withTrimmedRight (74), Justification::centredRight);
}

void DriftglassContent::drawMeters (Graphics& g, Rectangle<float> area)
{
    const auto& t = lnf.theme();
    auto labels = area.removeFromBottom (30);
    const float bw = (area.getWidth() - 10) / 3.0f;
    auto bar = [&] (Rectangle<float> r, float v, Colour col, const String& name, int index)
    {
        g.setColour (t.track);
        g.fillRoundedRectangle (r, 3);
        auto fill = r.withTrimmedTop (r.getHeight() * (1.0f - jlimit (0.0f, 1.0f, v)));
        g.setColour (col);
        g.fillRoundedRectangle (fill, 3);
        g.setColour (t.textDim);
        g.setFont (titleFont (9.5f));
        g.drawText (name, (int) (labels.getX() + index * (bw + 5)) - 4, (int) labels.getY() + 2, (int) bw + 8, 12, Justification::centred);
    };
    auto toMeter = [] (float lin) { return jlimit (0.0f, 1.0f, (float) (Decibels::gainToDecibels (lin, -60.0f) + 60.0f) / 60.0f); };
    bar (area.withWidth (bw), toMeter (inMeter), t.accent2, "IN", 0);
    bar (area.withWidth (bw).translated (bw + 5, 0), grMeter / 20.0f, t.accent.withAlpha (0.8f), "GR", 1);
    bar (area.withWidth (bw).translated (2 * (bw + 5), 0), toMeter (outMeter), t.accent, "OUT", 2);
    g.setColour (t.textDim);
    g.setFont (Font (FontOptions (10.0f)));
    g.drawText (String (grMeter, 1) + " dB", labels.withTrimmedTop (14).toNearestInt(), Justification::centred);
}

// ---------------------------------------------------------------- presets
void DriftglassContent::showPresetMenu()
{
    PopupMenu menu;
    const auto& ps = dg::factoryPresets();
    for (const auto& bank : dg::presetBanks())
    {
        PopupMenu sub;
        for (size_t i = 0; i < ps.size(); ++i)
            if (ps[i].bank == bank) sub.addItem ((int) i + 1, ps[i].name, true, (int) i == proc.getCurrentProgram() && proc.getPresetName() == String (ps[i].name));
        menu.addSubMenu (bank, sub);
    }
    const auto users = proc.userPresets();
    PopupMenu userMenu;
    for (int i = 0; i < users.size(); ++i) userMenu.addItem (10000 + i, users[i].getFileNameWithoutExtension());
    if (users.isEmpty()) userMenu.addItem (-1, "(none saved yet)", false);
    menu.addSubMenu ("User", userMenu);
    menu.addSeparator();
    menu.addItem (20000, "Save current as...");
    menu.addItem (20001, "Show preset folder");

    menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&presetButton).withMinimumWidth (presetButton.getWidth()),
                        [this, users] (int r)
                        {
                            if (r <= 0) return;
                            if (r < 10000) proc.loadFactoryPreset (r - 1);
                            else if (r < 20000) proc.loadUserPreset (users[r - 10000]);
                            else if (r == 20000) savePresetDialog();
                            else if (r == 20001)
                            {
                                DriftglassProcessor::userPresetFolder().createDirectory();
                                DriftglassProcessor::userPresetFolder().revealToUser();
                            }
                        });
}

void DriftglassContent::savePresetDialog()
{
    saveWindow = std::make_unique<AlertWindow> ("Save preset", "Name for this preset:", MessageBoxIconType::NoIcon, this);
    saveWindow->setLookAndFeel (&lnf);
    saveWindow->addTextEditor ("name", proc.getPresetName());
    saveWindow->addButton ("Save", 1, KeyPress (KeyPress::returnKey));
    saveWindow->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
    saveWindow->enterModalState (true, ModalCallbackFunction::create ([this] (int result)
    {
        if (result == 1 && saveWindow != nullptr) proc.saveUserPreset (saveWindow->getTextEditorContents ("name"));
        saveWindow.reset();
    }), false);
}

// ---------------------------------------------------------------- animation and live labels
void DriftglassContent::timerCallback()
{
    const auto& e = proc.getEngine();
    inMeter = e.inLevel.load();
    outMeter = e.outLevel.load();
    grMeter = grMeter * 0.7f + 0.3f * e.compGr.load();
    halo = halo * 0.85f + 0.15f * jlimit (0.0f, 1.0f, std::sqrt (e.verbLevel.load()) * 2.0f);
    phase += 0.06f;

    if (shownPreset != proc.getPresetName())
    {
        shownPreset = proc.getPresetName();
        const int idx = proc.getCurrentProgram();
        const auto& ps = dg::factoryPresets();
        String bank = "User";
        if (isPositiveAndBelow (idx, (int) ps.size()) && String (ps[(size_t) idx].name) == shownPreset) bank = ps[(size_t) idx].bank;
        presetButton.setButtonText (shownPreset + "   ·   " + bank);
    }

    if (charKnob != nullptr)
        charKnob->label = dg::characterLabel ((int) proc.apvts.getRawParameterValue ("verb_type")->load());
    if (timeKnob != nullptr)
        timeKnob->slider.setEnabled (proc.apvts.getRawParameterValue ("echo_sync")->load() < 0.5f);
    for (auto& m : modules)
    {
        const bool on = m->onParam < 0 || m->power.getToggleState();
        for (auto* k : m->knobs) k->slider.setAlpha (on ? 1.0f : 0.45f);
        m->type.setAlpha (on ? 1.0f : 0.5f);
        m->type2.setAlpha (on ? 1.0f : 0.5f);
    }
    repaint();
}

// ============================================================================ editor
DriftglassEditor::DriftglassEditor (DriftglassProcessor& p) : AudioProcessorEditor (p), proc (p), content (p)
{
    addAndMakeVisible (content);
    const double ratio = (double) DriftglassContent::W / DriftglassContent::H;
    setResizable (true, true);
    setResizeLimits ((int) (DriftglassContent::W * 0.7), (int) (DriftglassContent::H * 0.7), DriftglassContent::W * 2, DriftglassContent::H * 2);
    getConstrainer()->setFixedAspectRatio (ratio);
    const float s = jlimit (0.7f, 2.0f, proc.uiScale);
    setSize (roundToInt (DriftglassContent::W * s), roundToInt (DriftglassContent::H * s));
}

void DriftglassEditor::resized()
{
    // fit inside the editor even when a host ignores the aspect ratio
    const float s = jmin ((float) getWidth() / (float) DriftglassContent::W, (float) getHeight() / (float) DriftglassContent::H);
    content.setTransform (AffineTransform::scale (s).translated ((getWidth() - DriftglassContent::W * s) * 0.5f, 0.0f));
    content.setBounds (0, 0, DriftglassContent::W, DriftglassContent::H);
    proc.uiScale = s;
}
