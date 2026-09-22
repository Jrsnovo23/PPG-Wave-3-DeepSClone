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
      mod4Src (std::make_unique<ComboBoxSelector> (
