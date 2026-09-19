#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "UI/PPGLookAndFeel.h"
// ==================== InfoDisplay ====================

void PPGWave3Editor::InfoDisplay::setInfo (const juce::String& name, const juce::String& value)
{
    paramName  = name;
    paramValue = value;
    repaint();
}

void PPGWave3Editor::InfoDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (juce::Colour (0xffffaa00));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

    auto textArea = getLocalBounds().reduced (juce::roundToInt (5.0f * scale), 1);
    g.setColour (juce::Colour (0xffffcc55));
    g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(),
                                  juce::jmax (7.0f, 9.5f * scale), juce::Font::plain));

    if (paramValue.isEmpty())
    {
        g.drawText (paramName, textArea, juce::Justification::centredLeft);
    }
    else
    {
        auto nameArea = textArea.removeFromLeft (
            (int) ((float) textArea.getWidth() * 0.55f));
        g.drawText (paramName,  nameArea, juce::Justification::centredLeft);
        g.drawText (paramValue, textArea, juce::Justification::centredRight);
    }
}

// ==================== RotaryKnob ====================

PPGWave3Editor::RotaryKnob::RotaryKnob (juce::AudioProcessorValueTreeState& state,
                                        const juce::String& paramID,
                                        const juce::String& labelText,
                                        InfoDisplay* display)
    : infoDisplay (display), paramName (labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (0xffffaa00));
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff333333));
    slider.setColour (juce::Slider::thumbColourId,               juce::Colour (0xffffcc55));

    slider.onValueChange = [this]()
    {
        if (infoDisplay != nullptr)
            infoDisplay->setInfo (paramName, slider.getTextFromValue (slider.getValue()));
    };

    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffcccccc));
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        state, paramID, slider);
}

void PPGWave3Editor::RotaryKnob::setScale (float s)
{
    scale = s;
    label.setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scale)));
    resized();
}

void PPGWave3Editor::RotaryKnob::resized()
{
    auto r = getLocalBounds();
    const int labelH = juce::jmax (9, juce::roundToInt (11.0f * scale));
    label.setBounds (r.removeFromTop (labelH));
    slider.setBounds (r.reduced (2, 0));
}

void PPGWave3Editor::RotaryKnob::paint (juce::Graphics&) {}

// ==================== HSlider ====================

PPGWave3Editor::HSlider::HSlider (juce::AudioProcessorValueTreeState& state,
                                  const juce::String& paramID,
                                  InfoDisplay* display)
    : infoDisplay (display)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::trackColourId,       juce::Colour (0xffffaa00));
    slider.setColour (juce::Slider::backgroundColourId,  juce::Colour (0xff2a2a2a));
    slider.setColour (juce::Slider::thumbColourId,       juce::Colour (0xffffcc55));
    addAndMakeVisible (slider);

    slider.onValueChange = [this]()
    {
        if (infoDisplay != nullptr)
            infoDisplay->setInfo ("Mod Amount", slider.getTextFromValue (slider.getValue()));
    };

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        state, paramID, slider);
}

void PPGWave3Editor::HSlider::setScale (float s)
{
    scale = s;
    resized();
}

void PPGWave3Editor::HSlider::resized()
{
    slider.setBounds (getLocalBounds().reduced (2, 4));
}

void PPGWave3Editor::HSlider::paint (juce::Graphics&) {}

// ==================== ButtonSelector ====================

PPGWave3Editor::ButtonSelector::ButtonSelector (juce::AudioProcessorValueTreeState& state,
                                                const juce::String& paramID,
                                                const juce::StringArray& names)
    : apvtsRef (state), id (paramID)
{
    for (int i = 0; i < names.size(); ++i)
    {
        auto* b = buttons.add (new juce::TextButton (names[i]));
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff2a2a2a));
        b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        b->setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffcccccc));
        b->setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        b->onClick = [this, i]()
        {
            if (attachment)
                attachment->setValueAsCompleteGesture ((float) i);
        };
        addAndMakeVisible (b);
    }

    attachment = std::make_unique<juce::ParameterAttachment> (
        *apvtsRef.getParameter (id),
        [this] (float newValue)
        {
            currentIndex = (int) newValue;
            refreshFromParameter();
        });
    attachment->sendInitialUpdate();
}

void PPGWave3Editor::ButtonSelector::refreshFromParameter()
{
    for (int i = 0; i < buttons.size(); ++i)
        buttons[i]->setToggleState (i == currentIndex, juce::dontSendNotification);
}

void PPGWave3Editor::ButtonSelector::resized()
{
    auto r = getLocalBounds();
    const int w = r.getWidth() / juce::jmax (1, buttons.size());
    for (auto* b : buttons)
        b->setBounds (r.removeFromLeft (w).reduced (1));
}

void PPGWave3Editor::ButtonSelector::paint (juce::Graphics&) {}

// ==================== ComboBoxSelector ====================

PPGWave3Editor::ComboBoxSelector::ComboBoxSelector (juce::AudioProcessorValueTreeState& state,
                                                    const juce::String& paramID,
                                                    const juce::String& labelText,
                                                    InfoDisplay* display)
    : infoDisplay (display), paramName (labelText)
{
    combo.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff2a2a2a));
    combo.setColour (juce::ComboBox::textColourId,       juce::Colour (0xffffcc55));
    combo.setColour (juce::ComboBox::outlineColourId,    juce::Colour (0xff555555));
    combo.setColour (juce::ComboBox::arrowColourId,      juce::Colour (0xffffaa00));
    addAndMakeVisible (combo);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centredLeft);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffcccccc));
    addAndMakeVisible (label);

    if (auto* param = state.getParameter (paramID))
    {
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (param))
            combo.addItemList (choice->choices, 1);
    }

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        state, paramID, combo);

    combo.onChange = [this]()
    {
        if (infoDisplay != nullptr)
            infoDisplay->setInfo (paramName, combo.getText());
    };
}

void PPGWave3Editor::ComboBoxSelector::setScale (float s)
{
    scale = s;
    label.setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scale)));
    resized();
}

void PPGWave3Editor::ComboBoxSelector::resized()
{
    auto r = getLocalBounds();
    const int labelH = juce::jmax (9, juce::roundToInt (11.0f * scale));
    label.setBounds (r.removeFromTop (labelH));
    combo.setBounds (r);
}

void PPGWave3Editor::ComboBoxSelector::paint (juce::Graphics&) {}

// ==================== ToggleButton ====================

PPGWave3Editor::ToggleButton::ToggleButton (juce::AudioProcessorValueTreeState& state,
                                            const juce::String& paramID,
                                            const juce::String& labelText,
                                            InfoDisplay* display)
    : infoDisplay (display), paramName (labelText)
{
    button.setButtonText (labelText);
    button.setClickingTogglesState (true);
    button.setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff2a2a2a));
    button.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
    button.setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffcccccc));
    button.setColour (juce::TextButton::textColourOnId,   juce::Colours::black);

    button.onClick = [this]()
    {
        if (infoDisplay != nullptr)
            infoDisplay->setInfo (paramName, button.getToggleState() ? "ON" : "OFF");
    };

    addAndMakeVisible (button);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        state, paramID, button);
}

void PPGWave3Editor::ToggleButton::setScale (float s)
{
    scale = s;
}

void PPGWave3Editor::ToggleButton::resized()
{
    button.setBounds (getLocalBounds());
}

void PPGWave3Editor::ToggleButton::paint (juce::Graphics&) {}

// ==================== Editor ====================

PPGWave3Editor::PPGWave3Editor (PPGWave3Processor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      apvts (p.apvts),
      osc1Wave (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::osc1Wave,
                  juce::StringArray { "SIN", "TRI", "SAW", "SQR" })),
      osc1Pos   (p.apvts, ParamIDs::osc1Pos,    "POS",    &osc1Info),
      osc1Oct   (p.apvts, ParamIDs::osc1Octave, "OCT",    &osc1Info),
      osc1Semi  (p.apvts, ParamIDs::osc1Semi,   "SEMI",   &osc1Info),
      osc1Fine  (p.apvts, ParamIDs::osc1Fine,   "FINE",   &osc1Info),
      osc1Level (p.apvts, ParamIDs::osc1Level,  "LEVEL",  &osc1Info),
      osc2Wave (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::osc2Wave,
                  juce::StringArray { "SIN", "TRI", "SAW", "SQR" })),
      osc2Pos   (p.apvts, ParamIDs::osc2Pos,    "POS",    &osc2Info),
      osc2Oct   (p.apvts, ParamIDs::osc2Octave, "OCT",    &osc2Info),
      osc2Semi  (p.apvts, ParamIDs::osc2Semi,   "SEMI",   &osc2Info),
      osc2Fine  (p.apvts, ParamIDs::osc2Fine,   "FINE",   &osc2Info),
      osc2Level (p.apvts, ParamIDs::osc2Level,  "LEVEL",  &osc2Info),
      filterType (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::filterType,
                    juce::StringArray { "LP", "HP", "BP" })),
      filterCutoff  (p.apvts, ParamIDs::filterCutoff,   "CUTOFF",  &filterInfo),
      filterReso    (p.apvts, ParamIDs::filterReso,     "RESO",    &filterInfo),
      filterEnvAmt  (p.apvts, ParamIDs::filterEnvAmt,   "ENV AMT", &filterInfo),
      filterKeyTrack(p.apvts, ParamIDs::filterKeyTrack, "KEY TRK", &filterInfo),
      ampA (p.apvts, ParamIDs::ampAttack,  "A", &ampEnvInfo),
      ampD (p.apvts, ParamIDs::ampDecay,   "D", &ampEnvInfo),
      ampS (p.apvts, ParamIDs::ampSustain, "S", &ampEnvInfo),
      ampR (p.apvts, ParamIDs::ampRelease, "R", &ampEnvInfo),
      filtA (p.apvts, ParamIDs::filtAttack,  "A", &filtEnvInfo),
      filtD (p.apvts, ParamIDs::filtDecay,   "D", &filtEnvInfo),
      filtS (p.apvts, ParamIDs::filtSustain, "S", &filtEnvInfo),
      filtR (p.apvts, ParamIDs::filtRelease, "R", &filtEnvInfo),
      master (p.apvts, ParamIDs::masterGain, "MASTER", &masterInfo),
      lfo1Wave (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo1Wave, "WAVE", &lfoInfo)),
      lfo1Rate  (p.apvts, ParamIDs::lfo1Rate,  "RATE",  &lfoInfo),
      lfo1Depth (p.apvts, ParamIDs::lfo1Depth, "DEPTH", &lfoInfo),
      lfo1Phase (p.apvts, ParamIDs::lfo1Phase, "PHASE", &lfoInfo),
      lfo1Sync  (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo1Sync, "SYNC", &lfoInfo)),
      lfo2Wave (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo2Wave, "WAVE", &lfoInfo)),
      lfo2Rate  (p.apvts, ParamIDs::lfo2Rate,  "RATE",  &lfoInfo),
      lfo2Depth (p.apvts, ParamIDs::lfo2Depth, "DEPTH", &lfoInfo),
      lfo2Phase (p.apvts, ParamIDs::lfo2Phase, "PHASE", &lfoInfo),
      lfo2Sync  (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo2Sync, "SYNC", &lfoInfo)),
      mod1Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod1Source, "SRC", &modInfo)),
      mod1Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod1Dest,   "DST", &modInfo)),
      mod1Amt (p.apvts, ParamIDs::mod1Amount, &modInfo),
      mod2Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod2Source, "SRC", &modInfo)),
      mod2Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod2Dest,   "DST", &modInfo)),
      mod2Amt (p.apvts, ParamIDs::mod2Amount, &modInfo),
      mod3Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod3Source, "SRC", &modInfo)),
      mod3Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod3Dest,   "DST", &modInfo)),
      mod3Amt (p.apvts, ParamIDs::mod3Amount, &modInfo),
      mod4Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod4Source, "SRC", &modInfo)),
      mod4Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod4Dest,   "DST", &modInfo)),
      mod4Amt (p.apvts, ParamIDs::mod4Amount, &modInfo),
      driveOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::driveOn, "DRIVE", &fxInfo)),
      driveAmount (p.apvts, ParamIDs::driveAmount, "AMT",  &fxInfo),
      driveTone   (p.apvts, ParamIDs::driveTone,   "TONE", &fxInfo),
      driveMix    (p.apvts, ParamIDs::driveMix,    "MIX",  &fxInfo),
      chorusOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::chorusOn, "CHORUS", &fxInfo)),
      chorusRate  (p.apvts, ParamIDs::chorusRate,  "RATE",  &fxInfo),
      chorusDepth (p.apvts, ParamIDs::chorusDepth, "DEPTH", &fxInfo),
      chorusMix   (p.apvts, ParamIDs::chorusMix,   "MIX",   &fxInfo),
      delayOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::delayOn, "DELAY", &fxInfo)),
      delaySync (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::delaySync, "SYNC", &fxInfo)),
      delayTime     (p.apvts, ParamIDs::delayTime,     "TIME",  &fxInfo),
      delayFeedback (p.apvts, ParamIDs::delayFeedback, "FEEDBK",&fxInfo),
      delayMix      (p.apvts, ParamIDs::delayMix,      "MIX",   &fxInfo),
      reverbOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::reverbOn, "REVERB", &fxInfo)),
      reverbSize (p.apvts, ParamIDs::reverbSize, "SIZE", &fxInfo),
      reverbDamp (p.apvts, ParamIDs::reverbDamp, "DAMP", &fxInfo),
      reverbMix  (p.apvts, ParamIDs::reverbMix,  "MIX",  &fxInfo)
{
    juce::ignoreUnused (processorRef, apvts);

    addAndMakeVisible (*osc1Wave);
    addAndMakeVisible (*osc2Wave);
    addAndMakeVisible (*filterType);
    addAndMakeVisible (*lfo1Wave);   addAndMakeVisible (*lfo1Sync);
    addAndMakeVisible (*lfo2Wave);   addAndMakeVisible (*lfo2Sync);
    addAndMakeVisible (*mod1Src);    addAndMakeVisible (*mod1Dst);
    addAndMakeVisible (*mod2Src);    addAndMakeVisible (*mod2Dst);
    addAndMakeVisible (*mod3Src);    addAndMakeVisible (*mod3Dst);
    addAndMakeVisible (*mod4Src);    addAndMakeVisible (*mod4Dst);
    addAndMakeVisible (*driveOn);
    addAndMakeVisible (*chorusOn);
    addAndMakeVisible (*delayOn);    addAndMakeVisible (*delaySync);
    addAndMakeVisible (*reverbOn);

    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &ampEnvInfo, &filtEnvInfo, &masterInfo,
                     &lfoInfo, &modInfo, &fxInfo })
        addAndMakeVisible (d);

    std::initializer_list<juce::Component*> allKnobs {
        &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
        &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
        &filterCutoff, &filterReso, &filterEnvAmt, &filterKeyTrack,
        &ampA, &ampD, &ampS, &ampR,
        &filtA, &filtD, &filtS, &filtR,
        &master,
        &lfo1Rate, &lfo1Depth, &lfo1Phase,
        &lfo2Rate, &lfo2Depth, &lfo2Phase,
        &driveAmount, &driveTone, &driveMix,
        &chorusRate, &chorusDepth, &chorusMix,
        &delayTime, &delayFeedback, &delayMix,
        &reverbSize, &reverbDamp, &reverbMix,
        &mod1Amt, &mod2Amt, &mod3Amt, &mod4Amt
    };
    for (auto* c : allKnobs)
        addAndMakeVisible (c);

    // Aplicar LookAndFeel PPG
    setLookAndFeel (&ppgLnf);

    setResizable (true, true);
    setResizeLimits (900, 900, 1400, 1600);
    setSize (980, 1020);
}

PPGWave3Editor::~PPGWave3Editor()
{
    setLookAndFeel (nullptr);
}

float PPGWave3Editor::computeScale() const
{
    const float refH = 1020.0f;
    return juce::jlimit (0.72f, 1.5f, (float) getHeight() / refH);
}

void PPGWave3Editor::applyScaleToAll (float scaleValue)
{
    for (auto* k : { &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
                     &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
                     &filterCutoff, &filterReso, &filterEnvAmt, &filterKeyTrack,
                     &ampA, &ampD, &ampS, &ampR,
                     &filtA, &filtD, &filtS, &filtR,
                     &master,
                     &lfo1Rate, &lfo1Depth, &lfo1Phase,
                     &lfo2Rate, &lfo2Depth, &lfo2Phase,
                     &driveAmount, &driveTone, &driveMix,
                     &chorusRate, &chorusDepth, &chorusMix,
                     &delayTime, &delayFeedback, &delayMix,
                     &reverbSize, &reverbDamp, &reverbMix })
        k->setScale (scaleValue);

    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &ampEnvInfo, &filtEnvInfo, &masterInfo,
                     &lfoInfo, &modInfo, &fxInfo })
        d->setScale (scaleValue);

    for (auto* h : { &mod1Amt, &mod2Amt, &mod3Amt, &mod4Amt })
        h->setScale (scaleValue);

    osc1Wave  ->setScale (scaleValue);
    osc2Wave  ->setScale (scaleValue);
    filterType->setScale (scaleValue);

    lfo1Wave->setScale (scaleValue);  lfo2Wave->setScale (scaleValue);
    lfo1Sync->setScale (scaleValue);  lfo2Sync->setScale (scaleValue);
    delaySync->setScale (scaleValue);

    mod1Src->setScale (scaleValue);  mod1Dst->setScale (scaleValue);
    mod2Src->setScale (scaleValue);  mod2Dst->setScale (scaleValue);
    mod3Src->setScale (scaleValue);  mod3Dst->setScale (scaleValue);
    mod4Src->setScale (scaleValue);  mod4Dst->setScale (scaleValue);

    driveOn ->setScale (scaleValue);
    chorusOn->setScale (scaleValue);
    delayOn ->setScale (scaleValue);
    reverbOn->setScale (scaleValue);
}

void PPGWave3Editor::paint (juce::Graphics& g)
{
    // Fondo con degradado sutil
    g.fillAll (PPGLookAndFeel::bgApp());

    // Header con logo
    auto top = getLocalBounds().removeFromTop (36);
    {
        juce::ColourGradient grad (juce::Colour (0xff0a0a0a),
                                   0.0f, (float) top.getY(),
                                   juce::Colour (0xff1a1a1a),
                                   0.0f, (float) top.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRect (top);

        // Línea dorada inferior
        g.setColour (PPGLookAndFeel::accent());
        g.fillRect (top.getX(), top.getBottom() - 1, top.getWidth(), 1);

        // Logo
        drawLogo (g, top.reduced (10, 4));
    }

    drawSection (g, osc1Area,    "OSCILLATOR 1");
    drawSection (g, osc2Area,    "OSCILLATOR 2");
    drawSection (g, filterArea,  "FILTER");
    drawSection (g, lfoArea,     "LFO");
    drawSection (g, modArea,     "MODULATION MATRIX");
    drawSection (g, fxArea,      "EFFECTS");
    drawSection (g, ampEnvArea,  "AMP ENVELOPE");
    drawSection (g, filtEnvArea, "FILTER ENVELOPE");
    drawSection (g, masterArea,  "MASTER");
}

void PPGWave3Editor::drawSection (juce::Graphics& g, juce::Rectangle<int> area,
                                  const juce::String& title) const
{
    if (area.isEmpty()) return;

    // Fondo de panel con degradado sutil
    const auto r = area.toFloat();
    juce::ColourGradient grad (juce::Colour (0xff1c1c1c), r.getX(), r.getY(),
                               juce::Colour (0xff151515), r.getX(), r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (r, 4.0f);

    // Borde
    g.setColour (juce::Colour (0xff2f2f2f));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);

    // Título en dorado, mayúsculas
    g.setColour (PPGLookAndFeel::accent());
    g.setFont (juce::Font (juce::FontOptions (9.5f * currentScale, juce::Font::bold)));

    const int titleW = juce::jmin (area.getWidth() - 16, 240);
    const int titleY = area.getY() + 3;
    g.drawText (title.toUpperCase(), area.getX() + 8, titleY, titleW, 12,
                juce::Justification::centredLeft);

    // Línea dorada fina bajo el título
    g.setColour (PPGLookAndFeel::accent().withAlpha (0.35f));
    g.fillRect (area.getX() + 8, titleY + 13,
                juce::jmin (titleW, 200), 1);
}

void PPGWave3Editor::drawLogo (juce::Graphics& g, juce::Rectangle<int> area) const
{
    // "PPG" grande y bold + "WAVE 3.3" más fino al lado
    const float h = (float) area.getHeight();

    // PPG
    const float ppgSize = juce::jmax (18.0f, h * 0.75f * currentScale);
    g.setFont (juce::Font (juce::FontOptions (ppgSize, juce::Font::bold)));
    g.setColour (PPGLookAndFeel::accent());

    const juce::String ppgText ("PPG");
    const int ppgW = (int) g.getCurrentFont().getStringWidthFloat (ppgText) + 4;

    g.drawText (ppgText, area.removeFromLeft (ppgW),
                juce::Justification::centredLeft, false);

    // Espacio
    area.removeFromLeft (8);

    // WAVE 3.3
    const float waveSize = juce::jmax (10.0f, h * 0.42f * currentScale);
    g.setFont (juce::Font (juce::FontOptions (waveSize, juce::Font::plain)));
    g.setColour (PPGLookAndFeel::textPrimary());

    const juce::String waveText ("WAVE 3.3");
    const int waveW = (int) g.getCurrentFont().getStringWidthFloat (waveText) + 4;

    auto waveArea = area.removeFromLeft (waveW);
    g.drawText (waveText, waveArea, juce::Justification::centredLeft, false);

    // Subtítulo "Wave Table Synthesizer"
    area.removeFromLeft (8);
    g.setFont (juce::Font (juce::FontOptions (juce::jmax (8.0f, h * 0.28f * currentScale),
                                              juce::Font::italic)));
    g.setColour (PPGLookAndFeel::textDim());
    g.drawText ("Wave Table Synthesizer", area,
                juce::Justification::centredLeft, false);
}

void PPGWave3Editor::resized()
{
    currentScale = computeScale();
    applyScaleToAll (currentScale);

    auto r = getLocalBounds();
    r.removeFromTop (24);
    r.reduce (6, 6);

    const int h      = r.getHeight();
    const int gap    = 6;
    const int availH = h - 7 * gap;

    const float availHf = (float) availH;
    const int oscH  = (int) (availHf * 0.095f);
    const int filtH = (int) (availHf * 0.095f);
    const int lfoH  = (int) (availHf * 0.105f);
    const int modH  = (int) (availHf * 0.155f);
    const int fxH   = (int) (availHf * 0.265f);
    const int envH  = availH - 2 * oscH - filtH - lfoH - modH - fxH;

    osc1Area = r.removeFromTop (oscH);          r.removeFromTop (gap);
    osc2Area = r.removeFromTop (oscH);          r.removeFromTop (gap);
    filterArea = r.removeFromTop (filtH);       r.removeFromTop (gap);
    lfoArea = r.removeFromTop (lfoH);           r.removeFromTop (gap);
    modArea = r.removeFromTop (modH);           r.removeFromTop (gap);
    fxArea  = r.removeFromTop (fxH);            r.removeFromTop (gap);

    auto bottomRow = r.removeFromTop (envH);
    {
        const int totalW  = bottomRow.getWidth();
        const int masterW = (int) ((float) totalW * 0.14f);
        const int envW    = (totalW - masterW - 2 * gap) / 2;
        ampEnvArea  = bottomRow.removeFromLeft (envW);
        bottomRow.removeFromLeft (gap);
        filtEnvArea = bottomRow.removeFromLeft (envW);
        bottomRow.removeFromLeft (gap);
        masterArea  = bottomRow;
    }

    auto infoWidthFor = [] (int sectionW)
    {
        return juce::jlimit (90, 260, (int) ((float) sectionW * 0.22f));
    };

    auto titleRowFor = [&] (juce::Rectangle<int> area, InfoDisplay& info,
                            juce::Rectangle<int>& innerOut)
    {
        innerOut = area.reduced (8);
        auto titleRow = innerOut.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        info.setBounds (titleRow.removeFromRight (
            infoWidthFor (area.getWidth())).reduced (0, 1));
        innerOut.removeFromTop (1);
    };

    // -------- Osciladores --------
    auto layoutOscSection = [&] (juce::Rectangle<int> area,
                                 InfoDisplay& info, ButtonSelector& waveSel,
                                 RotaryKnob& kPos, RotaryKnob& kOct, RotaryKnob& kSemi,
                                 RotaryKnob& kFine, RotaryKnob& kLevel)
    {
        juce::Rectangle<int> inner;
        titleRowFor (area, info, inner);
        const int selW = (int) ((float) inner.getWidth() * 0.30f);
        waveSel.setBounds (inner.removeFromLeft (selW).reduced (2, 3));
        inner.removeFromLeft (4);
        const int kw = inner.getWidth() / 5;
        kPos  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        kOct  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        kSemi .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        kFine .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        kLevel.setBounds (inner.reduced (1, 0));
    };

    layoutOscSection (osc1Area, osc1Info, *osc1Wave,
                      osc1Pos, osc1Oct, osc1Semi, osc1Fine, osc1Level);
    layoutOscSection (osc2Area, osc2Info, *osc2Wave,
                      osc2Pos, osc2Oct, osc2Semi, osc2Fine, osc2Level);

    // -------- Filtro --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (filterArea, filterInfo, inner);
        const int selW = (int) ((float) inner.getWidth() * 0.22f);
        filterType->setBounds (inner.removeFromLeft (selW).reduced (2, 3));
        inner.removeFromLeft (4);
        const int kw = inner.getWidth() / 4;
        filterCutoff  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filterReso    .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filterEnvAmt  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filterKeyTrack.setBounds (inner.reduced (1, 0));
    }

    // -------- LFO --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (lfoArea, lfoInfo, inner);
        const int halfW = (inner.getWidth() - gap) / 2;

        auto layoutLFO = [&] (juce::Rectangle<int> area,
                              ComboBoxSelector& w, ComboBoxSelector& sync,
                              RotaryKnob& rate, RotaryKnob& depth, RotaryKnob& phase)
        {
            const int comboW = (int) ((float) area.getWidth() * 0.20f);
            w.setBounds (area.removeFromLeft (comboW).reduced (2, 0));
            area.removeFromLeft (2);
            sync.setBounds (area.removeFromLeft (comboW).reduced (2, 0));
            area.removeFromLeft (2);
            const int kw = area.getWidth() / 3;
            rate .setBounds (area.removeFromLeft (kw).reduced (1, 0));
            depth.setBounds (area.removeFromLeft (kw).reduced (1, 0));
            phase.setBounds (area.reduced (1, 0));
        };

        auto lfo1Zone = inner.removeFromLeft (halfW);
        inner.removeFromLeft (gap);
        auto lfo2Zone = inner;

        layoutLFO (lfo1Zone, *lfo1Wave, *lfo1Sync, lfo1Rate, lfo1Depth, lfo1Phase);
        layoutLFO (lfo2Zone, *lfo2Wave, *lfo2Sync, lfo2Rate, lfo2Depth, lfo2Phase);
    }

    // -------- Mod Matrix --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (modArea, modInfo, inner);
        const int rowH = inner.getHeight() / 4;

        auto layoutRow = [&] (juce::Rectangle<int> row,
                              ComboBoxSelector& src, ComboBoxSelector& dst, HSlider& amt)
        {
            const int srcW = (int) ((float) row.getWidth() * 0.35f);
            const int dstW = (int) ((float) row.getWidth() * 0.35f);
            src.setBounds (row.removeFromLeft (srcW).reduced (2, 1));
            dst.setBounds (row.removeFromLeft (dstW).reduced (2, 1));
            amt.setBounds (row.reduced (2, 1));
        };

        layoutRow (inner.removeFromTop (rowH), *mod1Src, *mod1Dst, mod1Amt);
        layoutRow (inner.removeFromTop (rowH), *mod2Src, *mod2Dst, mod2Amt);
        layoutRow (inner.removeFromTop (rowH), *mod3Src, *mod3Dst, mod3Amt);
        layoutRow (inner,                        *mod4Src, *mod4Dst, mod4Amt);
    }

    // -------- Effects 2x2 grid --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (fxArea, fxInfo, inner);

        const int rowH = inner.getHeight() / 2;
        const int colW = inner.getWidth() / 2;

        auto topRow = inner.removeFromTop (rowH);
        auto botRow = inner;

        auto topLeft  = topRow.removeFromLeft (colW);
        auto topRight = topRow;
        auto botLeft  = botRow.removeFromLeft (colW);
        auto botRight = botRow;

        const int toggleW = 68;

        auto layoutFxSimple = [] (juce::Rectangle<int> area, ToggleButton& on,
                                  RotaryKnob& k1, RotaryKnob& k2, RotaryKnob& k3,
                                  int toggleWidth)
        {
            area = area.reduced (4, 2);
            on.setBounds (area.removeFromLeft (toggleWidth).reduced (2, 6));
            area.removeFromLeft (2);
            const int kw = area.getWidth() / 3;
            k1.setBounds (area.removeFromLeft (kw).reduced (1, 0));
            k2.setBounds (area.removeFromLeft (kw).reduced (1, 0));
            k3.setBounds (area.reduced (1, 0));
        };

        // Top-Left: Drive
        layoutFxSimple (topLeft, *driveOn, driveAmount, driveTone, driveMix, toggleW);

        // Top-Right: Chorus
        layoutFxSimple (topRight, *chorusOn, chorusRate, chorusDepth, chorusMix, toggleW);

        // Bot-Left: Delay
        {
            auto area = botLeft.reduced (4, 2);
            delayOn->setBounds (area.removeFromLeft (toggleW).reduced (2, 6));
            area.removeFromLeft (2);
            const int syncW = (int) ((float) area.getWidth() * 0.22f);
            delaySync->setBounds (area.removeFromLeft (syncW).reduced (2, 0));
            area.removeFromLeft (2);
            const int kw = area.getWidth() / 3;
            delayTime    .setBounds (area.removeFromLeft (kw).reduced (1, 0));
            delayFeedback.setBounds (area.removeFromLeft (kw).reduced (1, 0));
            delayMix     .setBounds (area.reduced (1, 0));
        }

        // Bot-Right: Reverb
        layoutFxSimple (botRight, *reverbOn, reverbSize, reverbDamp, reverbMix, toggleW);
    }

    // -------- Amp Env --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (ampEnvArea, ampEnvInfo, inner);
        const int kw = inner.getWidth() / 4;
        ampA.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        ampD.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        ampS.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        ampR.setBounds (inner.reduced (1, 0));
    }

    // -------- Filter Env --------
    {
        juce::Rectangle<int> inner;
        titleRowFor (filtEnvArea, filtEnvInfo, inner);
        const int kw = inner.getWidth() / 4;
        filtA.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filtD.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filtS.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filtR.setBounds (inner.reduced (1, 0));
    }

    // -------- Master --------
    {
        auto inner = masterArea.reduced (8);
        auto titleRow = inner.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        masterInfo.setBounds (titleRow.reduced (0, 1));
        inner.removeFromTop (1);
        master.setBounds (inner.reduced (2, 0));
    }

    repaint();
}
