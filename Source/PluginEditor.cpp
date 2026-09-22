#include "PluginEditor.h"
#include "ParameterIDs.h"
#include "UI/PPGLookAndFeel.h"
#include "UI/Visualizers.h"

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
        auto nameArea = textArea.removeFromLeft ((int) ((float) textArea.getWidth() * 0.55f));
        g.drawText (paramName,  nameArea, juce::Justification::centredLeft);
        g.drawText (paramValue, textArea, juce::Justification::centredRight);
    }
}

// ==================== PresetDisplay ====================

void PPGWave3Editor::PresetDisplay::setInfo (const juce::String& name,
                                             const juce::String& category,
                                             bool factory)
{
    presetName     = name;
    presetCategory = category;
    isFactory      = factory;
    repaint();
}

void PPGWave3Editor::PresetDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();

    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (r, 3.0f);

    g.setColour (juce::Colour (0xffffaa00));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.0f, 1.0f);

    auto inner = getLocalBounds().reduced (6, 2);

    g.setColour (juce::Colour (0xffffcc55));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText (presetName, inner, juce::Justification::centred, false);

    auto catRow = inner.removeFromBottom (10);
    g.setColour (isFactory ? juce::Colour (0xffffaa00)
                            : juce::Colour (0xff2ecc40));
    g.setFont (juce::FontOptions (8.0f, juce::Font::plain));
    const auto tag = isFactory ? "FACTORY" : "USER";
    g.drawText (presetCategory.toUpperCase() + "  -  " + tag,
                catRow, juce::Justification::centred, false);
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

// ==================== EQBandKnob (FASE 6.7) ====================

PPGWave3Editor::EQBandKnob::EQBandKnob (juce::AudioProcessorValueTreeState& state,
                                        const juce::StringArray& paramIdsForBands,
                                        const juce::String& labelText,
                                        InfoDisplay* display)
    : apvtsRef (state), ids (paramIdsForBands),
      infoDisplay (display), paramName (labelText)
{
    jassert (ids.size() == 4);

    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (0xffffaa00));
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff333333));
    slider.setColour (juce::Slider::thumbColourId,               juce::Colour (0xffffcc55));

    // Rango temporal: lo ajustamos según el parámetro activo en refreshSliderFromParam.
    slider.setRange (0.0, 1.0, 0.0001);

    slider.onValueChange = [this]() { sliderChanged(); };

    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffcccccc));
    addAndMakeVisible (label);

    refreshSliderFromParam();
    startTimerHz (30);
}

PPGWave3Editor::EQBandKnob::~EQBandKnob()
{
    stopTimer();
}

void PPGWave3Editor::EQBandKnob::setActiveBand (int band)
{
    activeBand = juce::jlimit (0, 3, band);
    refreshSliderFromParam();
}

void PPGWave3Editor::EQBandKnob::setScale (float s)
{
    scale = s;
    label.setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scale)));
    resized();
}

void PPGWave3Editor::EQBandKnob::resized()
{
    auto r = getLocalBounds();
    const int labelH = juce::jmax (9, juce::roundToInt (11.0f * scale));
    label.setBounds (r.removeFromTop (labelH));
    slider.setBounds (r.reduced (2, 0));
}

void PPGWave3Editor::EQBandKnob::paint (juce::Graphics&) {}

void PPGWave3Editor::EQBandKnob::refreshSliderFromParam()
{
    const auto id = ids[activeBand];
    auto* param = apvtsRef.getParameter (id);
    if (param == nullptr) return;

    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
    {
        const auto& range = ranged->getNormalisableRange();
        slider.setRange (range.start, range.end, range.interval > 0.0f
                                                       ? range.interval
                                                       : 0.0001);

        // Si el rango es muy amplio, aplicamos skew para que sea cómodo.
        // (Se pierde la simetría log original del parámetro, pero para
        //  el EQ es aceptable y mucho más simple.)
        if (range.end / juce::jmax (0.001f, range.start) > 100.0f)
        {
            auto skewed = juce::NormalisableRange<double> (range.start, range.end);
            skewed.setSkewForCentre (std::sqrt ((double) range.start * (double) range.end));
            slider.setNormalisableRange (skewed);
        }
        else
        {
            slider.setNormalisableRange (juce::NormalisableRange<double> (
                range.start, range.end));
        }
    }

    updatingFromParam = true;
    if (auto* raw = apvtsRef.getRawParameterValue (id))
        slider.setValue (raw->load(), juce::dontSendNotification);
    updatingFromParam = false;
}

void PPGWave3Editor::EQBandKnob::sliderChanged()
{
    if (updatingFromParam) return;

    const auto id = ids[activeBand];
    auto* param = apvtsRef.getParameter (id);
    if (param == nullptr) return;

    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
    {
        const auto& range = ranged->getNormalisableRange();
        const float normalized = range.convertTo0to1 ((float) slider.getValue());
        param->setValueNotifyingHost (normalized);

        if (infoDisplay != nullptr)
            infoDisplay->setInfo (paramName, slider.getTextFromValue (slider.getValue()));
    }
}

void PPGWave3Editor::EQBandKnob::timerCallback()
{
    const auto id = ids[activeBand];
    if (auto* raw = apvtsRef.getRawParameterValue (id))
    {
        const float real = raw->load();
        if (std::abs (real - (float) slider.getValue()) > 0.0001f)
        {
            updatingFromParam = true;
            slider.setValue (real, juce::dontSendNotification);
            updatingFromParam = false;
        }
    }
}

// ==================== Editor ====================

PPGWave3Editor::PPGWave3Editor (PPGWave3Processor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      apvts (p.apvts),
      presetManager (p.apvts),
      prevBtn ("<"),
      nextBtn (">"),
      loadBtn ("LOAD"),
      saveBtn ("SAVE"),
      browseBtn ("BROWSE"),
      osc1Wave (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::osc1Wave,
                  juce::StringArray { "SIN", "TRI", "SAW", "SQR" })),
      osc1Preview (p.apvts, ParamIDs::osc1Wave, ParamIDs::osc1Pos),
      osc1Pos   (p.apvts, ParamIDs::osc1Pos,    "POS",    &osc1Info),
      osc1Oct   (p.apvts, ParamIDs::osc1Octave, "OCT",    &osc1Info),
      osc1Semi  (p.apvts, ParamIDs::osc1Semi,   "SEMI",   &osc1Info),
      osc1Fine  (p.apvts, ParamIDs::osc1Fine,   "FINE",   &osc1Info),
      osc1Level (p.apvts, ParamIDs::osc1Level,  "LEVEL",  &osc1Info),
      osc2Wave (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::osc2Wave,
                  juce::StringArray { "SIN", "TRI", "SAW", "SQR" })),
      osc2Preview (p.apvts, ParamIDs::osc2Wave, ParamIDs::osc2Pos),
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
      env1Display (p.apvts, ParamIDs::ampAttack, ParamIDs::ampDecay,
                   ParamIDs::ampSustain, ParamIDs::ampRelease),
      env2Display (p.apvts, ParamIDs::filtAttack, ParamIDs::filtDecay,
                   ParamIDs::filtSustain, ParamIDs::filtRelease),
      env3Display (p.apvts, ParamIDs::env3Attack, ParamIDs::env3Decay,
                   ParamIDs::env3Sustain, ParamIDs::env3Release),
      env1A (p.apvts, ParamIDs::ampAttack,   "A", &envInfo),
      env1D (p.apvts, ParamIDs::ampDecay,    "D", &envInfo),
      env1S (p.apvts, ParamIDs::ampSustain,  "S", &envInfo),
      env1R (p.apvts, ParamIDs::ampRelease,  "R", &envInfo),
      env2A (p.apvts, ParamIDs::filtAttack,  "A", &envInfo),
      env2D (p.apvts, ParamIDs::filtDecay,   "D", &envInfo),
      env2S (p.apvts, ParamIDs::filtSustain, "S", &envInfo),
      env2R (p.apvts, ParamIDs::filtRelease, "R", &envInfo),
      env3A (p.apvts, ParamIDs::env3Attack,  "A", &envInfo),
      env3D (p.apvts, ParamIDs::env3Decay,   "D", &envInfo),
      env3S (p.apvts, ParamIDs::env3Sustain, "S", &envInfo),
      env3R (p.apvts, ParamIDs::env3Release, "R", &envInfo),
      master (p.apvts, ParamIDs::masterGain, "MASTER", &masterInfo),
      masterMeter (p.peakLevel),
      lfo1Wave (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo1Wave, "WAVE", &lfoInfo)),
      lfo1Display (p.apvts, ParamIDs::lfo1Wave),
      lfo1Rate  (p.apvts, ParamIDs::lfo1Rate,  "RATE",  &lfoInfo),
      lfo1Depth (p.apvts, ParamIDs::lfo1Depth, "DEPTH", &lfoInfo),
      lfo1Phase (p.apvts, ParamIDs::lfo1Phase, "PHASE", &lfoInfo),
      lfo1Sync  (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo1Sync, "SYNC", &lfoInfo)),
      lfo2Wave (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo2Wave, "WAVE", &lfoInfo)),
      lfo2Display (p.apvts, ParamIDs::lfo2Wave),
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
      reverbMix  (p.apvts, ParamIDs::reverbMix,  "MIX",  &fxInfo),
      // ===== FASE 6.7: EQ =====
      eqOn   (std::make_unique<ToggleButton> (p.apvts, ParamIDs::eqOn,   "EQ", &fxInfo)),
      eqHpOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::eqHpOn, "HP", &fxInfo)),
      eqLpOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::eqLpOn, "LP", &fxInfo)),
      eqFreqKnob (p.apvts,
                  { ParamIDs::eqLowFreq, ParamIDs::eqLmidFreq,
                    ParamIDs::eqHmidFreq, ParamIDs::eqHighFreq },
                  "FREQ", &fxInfo),
      eqQKnob    (p.apvts,
                  { ParamIDs::eqLowQ, ParamIDs::eqLmidQ,
                    ParamIDs::eqHmidQ, ParamIDs::eqHighQ },
                  "Q", &fxInfo),
      eqGainKnob (p.apvts,
                  { ParamIDs::eqLowGain, ParamIDs::eqLmidGain,
                    ParamIDs::eqHmidGain, ParamIDs::eqHighGain },
                  "GAIN", &fxInfo),
      // ===== FASE 6.7: Phaser =====
      phaserOn (std::make_unique<ToggleButton> (p.apvts, ParamIDs::phaserOn, "PHASER", &fxInfo)),
      phaserRate     (p.apvts, ParamIDs::phaserRate,     "RATE",  &fxInfo),
      phaserDepth    (p.apvts, ParamIDs::phaserDepth,    "DEPTH", &fxInfo),
      phaserFeedback (p.apvts, ParamIDs::phaserFeedback, "FEEDBK",&fxInfo),
      phaserMix      (p.apvts, ParamIDs::phaserMix,      "MIX",   &fxInfo),
      keyboardComponent (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    juce::ignoreUnused (processorRef, apvts);

    // === Preset bar ===
    prevBtn.setConnectedEdges (juce::Button::ConnectedOnRight);
    nextBtn.setConnectedEdges (juce::Button::ConnectedOnLeft);
    prevBtn.onClick = [this]() { onPrevPreset(); };
    nextBtn.onClick = [this]() { onNextPreset(); };
    loadBtn.onClick = [this]() { onLoadPreset(); };
    saveBtn.onClick = [this]() { onSavePreset(); };
    browseBtn.onClick = [this]() { onBrowsePreset(); };

    for (auto* b : { &prevBtn, &nextBtn, &loadBtn, &saveBtn, &browseBtn })
        addAndMakeVisible (b);

    addAndMakeVisible (presetDisplay);
    updatePresetDisplay();

    // === FX tabs ===
    for (auto* b : { &driveTabBtn, &chorusTabBtn, &delayTabBtn,
                     &reverbTabBtn, &eqTabBtn, &phaserTabBtn })
    {
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff1c1c1c));
        b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        b->setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffaaaaaa));
        b->setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        addAndMakeVisible (b);
    }

    driveTabBtn .setButtonText ("DIST");
    chorusTabBtn.setButtonText ("CHORUS");
    delayTabBtn .setButtonText ("DELAY");
    reverbTabBtn.setButtonText ("REVERB");
    eqTabBtn    .setButtonText ("EQ");
    phaserTabBtn.setButtonText ("PHASER");

    driveTabBtn .onClick = [this]() { activeFxTab = 0; updateFxVisibility(); repaint(); };
    chorusTabBtn.onClick = [this]() { activeFxTab = 1; updateFxVisibility(); repaint(); };
    delayTabBtn .onClick = [this]() { activeFxTab = 2; updateFxVisibility(); repaint(); };
    reverbTabBtn.onClick = [this]() { activeFxTab = 3; updateFxVisibility(); repaint(); };
    eqTabBtn    .onClick = [this]() { activeFxTab = 4; updateFxVisibility(); repaint(); };
    phaserTabBtn.onClick = [this]() { activeFxTab = 5; updateFxVisibility(); repaint(); };

    // === Env tabs ===
    for (auto* b : { &env1TabBtn, &env2TabBtn, &env3TabBtn })
    {
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff1c1c1c));
        b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        b->setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffaaaaaa));
        b->setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        addAndMakeVisible (b);
    }

    env1TabBtn.setButtonText ("ENV1 - AMP");
    env2TabBtn.setButtonText ("ENV2 - FILTER");
    env3TabBtn.setButtonText ("ENV3 - FREE");

    env1TabBtn.onClick = [this]() { activeEnvTab = 0; updateEnvVisibility(); repaint(); };
    env2TabBtn.onClick = [this]() { activeEnvTab = 1; updateEnvVisibility(); repaint(); };
    env3TabBtn.onClick = [this]() { activeEnvTab = 2; updateEnvVisibility(); repaint(); };

    // === FASE 6.7: Botones de banda del EQ ===
    for (auto* b : { &eqLowBtn, &eqLmidBtn, &eqHmidBtn, &eqHighBtn })
    {
        b->setClickingTogglesState (true);
        b->setRadioGroupId (0xE0B);
        b->setColour (juce::TextButton::buttonColourId,   juce::Colour (0xff2a2a2a));
        b->setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffffaa00));
        b->setColour (juce::TextButton::textColourOffId,  juce::Colour (0xffcccccc));
        b->setColour (juce::TextButton::textColourOnId,   juce::Colours::black);
        addAndMakeVisible (b);
    }
    eqLowBtn .setButtonText ("LOW");
    eqLmidBtn.setButtonText ("LOW MID");
    eqHmidBtn.setButtonText ("HI MID");
    eqHighBtn.setButtonText ("HIGH");

    eqLowBtn .onClick = [this]() { setActiveEqBand (0); };
    eqLmidBtn.onClick = [this]() { setActiveEqBand (1); };
    eqHmidBtn.onClick = [this]() { setActiveEqBand (2); };
    eqHighBtn.onClick = [this]() { setActiveEqBand (3); };

    setActiveEqBand (0);   // default LOW

    // === Teclado virtual ===
    keyboardComponent.setAvailableRange (36, 96);
    keyboardComponent.setLowestVisibleKey (36);
    keyboardComponent.setKeyWidth (20.0f);
    keyboardComponent.setScrollButtonsVisible (false);
    keyboardComponent.setColour (juce::MidiKeyboardComponent::whiteNoteColourId,
                                 juce::Colour (0xffe8e8e8));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::blackNoteColourId,
                                 juce::Colour (0xff1a1a1a));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId,
                                 juce::Colour (0xff2f2f2f));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId,
                                 juce::Colour (0xffffaa00).withAlpha (0.35f));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId,
                                 juce::Colour (0xffffaa00).withAlpha (0.75f));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::shadowColourId,
                                 juce::Colour (0x66000000));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::textLabelColourId,
                                 juce::Colour (0xff444444));
    addAndMakeVisible (keyboardComponent);

    // === Pitch wheel ===
    pitchWheelSlider.setSliderStyle (juce::Slider::LinearVertical);
    pitchWheelSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    pitchWheelSlider.setRange (-1.0, 1.0, 0.001);
    pitchWheelSlider.setValue (0.0, juce::dontSendNotification);
    pitchWheelSlider.setColour (juce::Slider::trackColourId,      juce::Colour (0xffffaa00));
    pitchWheelSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a2a2a));
    pitchWheelSlider.setColour (juce::Slider::thumbColourId,      juce::Colour (0xffffcc55));
    pitchWheelSlider.onValueChange = [this]()
    {
        processorRef.pitchBendAtomic.store ((float) pitchWheelSlider.getValue());
    };
    pitchWheelSlider.onDragEnd = [this]()
    {
        pitchWheelSlider.setValue (0.0, juce::sendNotificationSync);
    };
    addAndMakeVisible (pitchWheelSlider);

    pitchWheelLabel.setText ("PITCH", juce::dontSendNotification);
    pitchWheelLabel.setJustificationType (juce::Justification::centred);
    pitchWheelLabel.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
    pitchWheelLabel.setFont (juce::FontOptions (9.0f, juce::Font::bold));
    addAndMakeVisible (pitchWheelLabel);

    // === Mod wheel ===
    modWheelSlider.setSliderStyle (juce::Slider::LinearVertical);
    modWheelSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    modWheelSlider.setRange (0.0, 1.0, 0.001);
    modWheelSlider.setValue (0.0, juce::dontSendNotification);
    modWheelSlider.setColour (juce::Slider::trackColourId,      juce::Colour (0xffffaa00));
    modWheelSlider.setColour (juce::Slider::backgroundColourId, juce::Colour (0xff2a2a2a));
    modWheelSlider.setColour (juce::Slider::thumbColourId,      juce::Colour (0xffffcc55));
    modWheelSlider.onValueChange = [this]()
    {
        processorRef.modWheelAtomic.store ((float) modWheelSlider.getValue());
    };
    addAndMakeVisible (modWheelSlider);

    modWheelLabel.setText ("MOD", juce::dontSendNotification);
    modWheelLabel.setJustificationType (juce::Justification::centred);
    modWheelLabel.setColour (juce::Label::textColourId, juce::Colour (0xffaaaaaa));
    modWheelLabel.setFont (juce::FontOptions (9.0f, juce::Font::bold));
    addAndMakeVisible (modWheelLabel);

    // === Visualizadores ===
    addAndMakeVisible (osc1Preview);
    addAndMakeVisible (osc2Preview);
    addAndMakeVisible (lfo1Display);
    addAndMakeVisible (lfo2Display);
    addAndMakeVisible (env1Display);
    addAndMakeVisible (env2Display);
    addAndMakeVisible (env3Display);
    addAndMakeVisible (masterMeter);

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
    addAndMakeVisible (*eqOn);
    addAndMakeVisible (*eqHpOn);
    addAndMakeVisible (*eqLpOn);
    addAndMakeVisible (*phaserOn);

    addAndMakeVisible (eqFreqKnob);
    addAndMakeVisible (eqQKnob);
    addAndMakeVisible (eqGainKnob);

    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &envInfo, &masterInfo,
                     &lfoInfo, &modInfo, &fxInfo })
        addAndMakeVisible (d);

    std::initializer_list<juce::Component*> allKnobs {
        &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
        &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
        &filterCutoff, &filterReso, &filterEnvAmt, &filterKeyTrack,
        &env1A, &env1D, &env1S, &env1R,
        &env2A, &env2D, &env2S, &env2R,
        &env3A, &env3D, &env3S, &env3R,
        &master,
        &lfo1Rate, &lfo1Depth, &lfo1Phase,
        &lfo2Rate, &lfo2Depth, &lfo2Phase,
        &driveAmount, &driveTone, &driveMix,
        &chorusRate, &chorusDepth, &chorusMix,
        &delayTime, &delayFeedback, &delayMix,
        &reverbSize, &reverbDamp, &reverbMix,
        &phaserRate, &phaserDepth, &phaserFeedback, &phaserMix,
        &mod1Amt, &mod2Amt, &mod3Amt, &mod4Amt
    };
    for (auto* c : allKnobs)
        addAndMakeVisible (c);

    setLookAndFeel (&ppgLnf);

    setResizable (false, false);
    setSize (1280, 1040);

    updateFxVisibility();
    updateEnvVisibility();
}

PPGWave3Editor::~PPGWave3Editor()
{
    setLookAndFeel (nullptr);
}

// ==================== EQ band switching ====================

void PPGWave3Editor::setActiveEqBand (int band)
{
    activeEqBand = juce::jlimit (0, 3, band);
    eqFreqKnob.setActiveBand (activeEqBand);
    eqQKnob   .setActiveBand (activeEqBand);
    eqGainKnob.setActiveBand (activeEqBand);

    eqLowBtn .setToggleState (activeEqBand == 0, juce::dontSendNotification);
    eqLmidBtn.setToggleState (activeEqBand == 1, juce::dontSendNotification);
    eqHmidBtn.setToggleState (activeEqBand == 2, juce::dontSendNotification);
    eqHighBtn.setToggleState (activeEqBand == 3, juce::dontSendNotification);
}

// ==================== FX Tab visibility ====================

void PPGWave3Editor::updateFxVisibility()
{
    const bool d  = (activeFxTab == 0);
    const bool c  = (activeFxTab == 1);
    const bool dl = (activeFxTab == 2);
    const bool r  = (activeFxTab == 3);
    const bool eq = (activeFxTab == 4);
    const bool ph = (activeFxTab == 5);

    driveOn    ->setVisible (d);
    driveAmount.setVisible (d);
    driveTone  .setVisible (d);
    driveMix   .setVisible (d);

    chorusOn   ->setVisible (c);
    chorusRate .setVisible (c);
    chorusDepth.setVisible (c);
    chorusMix  .setVisible (c);

    delayOn    ->setVisible (dl);
    delaySync  ->setVisible (dl);
    delayTime  .setVisible (dl);
    delayFeedback.setVisible (dl);
    delayMix   .setVisible (dl);

    reverbOn   ->setVisible (r);
    reverbSize .setVisible (r);
    reverbDamp .setVisible (r);
    reverbMix  .setVisible (r);

    eqOn   ->setVisible (eq);
    eqHpOn ->setVisible (eq);
    eqLpOn ->setVisible (eq);

    eqLowBtn .setVisible (eq);
    eqLmidBtn.setVisible (eq);
    eqHmidBtn.setVisible (eq);
    eqHighBtn.setVisible (eq);

    eqFreqKnob.setVisible (eq);
    eqQKnob   .setVisible (eq);
    eqGainKnob.setVisible (eq);

    phaserOn      ->setVisible (ph);
    phaserRate    .setVisible (ph);
    phaserDepth   .setVisible (ph);
    phaserFeedback.setVisible (ph);
    phaserMix     .setVisible (ph);

    driveTabBtn .setToggleState (d,  juce::dontSendNotification);
    chorusTabBtn.setToggleState (c,  juce::dontSendNotification);
    delayTabBtn .setToggleState (dl, juce::dontSendNotification);
    reverbTabBtn.setToggleState (r,  juce::dontSendNotification);
    eqTabBtn    .setToggleState (eq, juce::dontSendNotification);
    phaserTabBtn.setToggleState (ph, juce::dontSendNotification);
}

// ==================== Env Tab visibility ====================

void PPGWave3Editor::updateEnvVisibility()
{
    const bool e1 = (activeEnvTab == 0);
    const bool e2 = (activeEnvTab == 1);
    const bool e3 = (activeEnvTab == 2);

    env1Display.setVisible (e1);
    env1A.setVisible (e1); env1D.setVisible (e1);
    env1S.setVisible (e1); env1R.setVisible (e1);

    env2Display.setVisible (e2);
    env2A.setVisible (e2); env2D.setVisible (e2);
    env2S.setVisible (e2); env2R.setVisible (e2);

    env3Display.setVisible (e3);
    env3A.setVisible (e3); env3D.setVisible (e3);
    env3S.setVisible (e3); env3R.setVisible (e3);

    env1TabBtn.setToggleState (e1, juce::dontSendNotification);
    env2TabBtn.setToggleState (e2, juce::dontSendNotification);
    env3TabBtn.setToggleState (e3, juce::dontSendNotification);
}

// ==================== Preset actions ====================

void PPGWave3Editor::updatePresetDisplay()
{
    const int idx = presetManager.getCurrentIndex();
    const auto& list = presetManager.getAllPresets();

    if (idx >= 0 && idx < list.size())
    {
        const auto& info = list.getReference (idx);
        presetDisplay.setInfo (info.name, info.category, info.isFactory);
    }
    else
    {
        presetDisplay.setInfo (presetManager.getCurrentName(),
                               presetManager.getCurrentCategory(), true);
    }
}

void PPGWave3Editor::onPrevPreset()
{
    presetManager.prev();
    updatePresetDisplay();
}

void PPGWave3Editor::onNextPreset()
{
    presetManager.next();
    updatePresetDisplay();
}

void PPGWave3Editor::onLoadPreset()
{
    presetManager.refresh();
    presetManager.loadByIndex (presetManager.getCurrentIndex());
    updatePresetDisplay();
}

void PPGWave3Editor::onSavePreset()
{
    auto* window = new juce::AlertWindow ("Save Preset",
                                          "Enter preset name and category:",
                                          juce::MessageBoxIconType::NoIcon);
    window->addTextEditor ("name", presetManager.getCurrentName(), "Name:");
    window->addComboBox ("cat",
                         { "Bass","Lead","Pad","Keys","Bell","Pluck",
                           "Sequence","FX","Atmospheric","Digital",
                           "Experimental","Percussive" },
                         "Category:");
    window->addButton ("Save",   1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    window->enterModalState (true,
        juce::ModalCallbackFunction::create ([this, window] (int result)
        {
            if (result == 1)
            {
                auto name = window->getTextEditorContents ("name");
                auto* cb  = window->getComboBoxComponent ("cat");
                auto cat  = cb != nullptr ? cb->getText() : juce::String ("User");

                if (name.isNotEmpty())
                {
                    presetManager.saveUserPreset (name, cat);
                    updatePresetDisplay();
                }
            }
        }),
        true);
}

void PPGWave3Editor::onBrowsePreset()
{
    fileChooser = std::make_unique<juce::FileChooser> (
        "Load Preset",
        presetManager.getUserPresetDirectory(),
        "*.json");

    fileChooser->launchAsync (
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                presetManager.loadFromFile (file);
                updatePresetDisplay();
            }
        });
}

// ==================== Scale ====================

float PPGWave3Editor::computeScale() const
{
    const float refH = 1040.0f;
    return juce::jlimit (0.72f, 1.5f, (float) getHeight() / refH);
}

void PPGWave3Editor::applyScaleToAll (float scaleValue)
{
    for (auto* k : { &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
                     &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
                     &filterCutoff, &filterReso, &filterEnvAmt, &filterKeyTrack,
                     &env1A, &env1D, &env1S, &env1R,
                     &env2A, &env2D, &env2S, &env2R,
                     &env3A, &env3D, &env3S, &env3R,
                     &master,
                     &lfo1Rate, &lfo1Depth, &lfo1Phase,
                     &lfo2Rate, &lfo2Depth, &lfo2Phase,
                     &driveAmount, &driveTone, &driveMix,
                     &chorusRate, &chorusDepth, &chorusMix,
                     &delayTime, &delayFeedback, &delayMix,
                     &reverbSize, &reverbDamp, &reverbMix,
                     &phaserRate, &phaserDepth, &phaserFeedback, &phaserMix })
        k->setScale (scaleValue);

    eqFreqKnob.setScale (scaleValue);
    eqQKnob   .setScale (scaleValue);
    eqGainKnob.setScale (scaleValue);

    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &envInfo, &masterInfo,
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
    eqOn    ->setScale (scaleValue);
    eqHpOn  ->setScale (scaleValue);
    eqLpOn  ->setScale (scaleValue);
    phaserOn->setScale (scaleValue);

    pitchWheelLabel.setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scaleValue),
                                                juce::Font::bold));
    modWheelLabel  .setFont (juce::FontOptions (juce::jmax (7.0f, 9.0f * scaleValue),
                                                juce::Font::bold));
}

// ==================== paint ====================

void PPGWave3Editor::paint (juce::Graphics& g)
{
    g.fillAll (PPGLookAndFeel::bgApp());

    auto top = getLocalBounds().removeFromTop (72);
    {
        juce::ColourGradient grad (juce::Colour (0xff0a0a0a),
                                   0.0f, (float) top.getY(),
                                   juce::Colour (0xff1a1a1a),
                                   0.0f, (float) top.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRect (top);

        g.setColour (PPGLookAndFeel::accent());
        g.fillRect (top.getX(), top.getBottom() - 1, top.getWidth(), 1);

        auto logoRow = top.removeFromTop (36);
        drawLogo (g, logoRow.reduced (10, 4));
    }

    drawSection (g, osc1Area,    "OSCILLATOR 1");
    drawSection (g, osc2Area,    "OSCILLATOR 2");
    drawSection (g, filterArea,  "FILTER");
    drawSection (g, lfoArea,     "LFO");
    drawSection (g, modArea,     "MODULATION MATRIX");
    drawSection (g, fxArea,      "EFFECTS");
    drawSection (g, envArea,     "ENVELOPES");
    drawSection (g, masterArea,  "MASTER");

    if (! keyboardArea.isEmpty())
    {
        const auto r = keyboardArea.toFloat();
        juce::ColourGradient grad (juce::Colour (0xff1c1c1c), r.getX(), r.getY(),
                                   juce::Colour (0
