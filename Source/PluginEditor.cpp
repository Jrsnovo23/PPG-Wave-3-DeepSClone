#include "PluginEditor.h"
#include "ParameterIDs.h"

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

    auto textArea = getLocalBounds().reduced (5, 1);
    g.setColour (juce::Colour (0xffffcc55));
    g.setFont (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(),
                                  9.5f, juce::Font::plain));

    if (paramValue.isEmpty())
    {
        g.drawText (paramName, textArea, juce::Justification::centredLeft);
    }
    else
    {
        auto nameArea = textArea.removeFromLeft ((int) (textArea.getWidth() * 0.55f));
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
    label.setFont (juce::FontOptions (9.5f));
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        state, paramID, slider);
}

void PPGWave3Editor::RotaryKnob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (11));
    slider.setBounds (r.reduced (2, 0));
}

void PPGWave3Editor::RotaryKnob::paint (juce::Graphics&) {}

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
      master (p.apvts, ParamIDs::masterGain, "MASTER", &masterInfo)
{
    juce::ignoreUnused (processorRef, apvts);

    addAndMakeVisible (*osc1Wave);
    addAndMakeVisible (*osc2Wave);
    addAndMakeVisible (*filterType);

    // Info displays
    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &ampEnvInfo, &filtEnvInfo, &masterInfo })
        addAndMakeVisible (d);

    // Perillas
    std::initializer_list<juce::Component*> allKnobs {
        &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
        &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
        &filterCutoff, &filterReso, &filterEnvAmt, &filterKeyTrack,
        &ampA, &ampD, &ampS, &ampR,
        &filtA, &filtD, &filtS, &filtR,
        &master
    };
    for (auto* c : allKnobs)
        addAndMakeVisible (c);

    setResizable (true, true);
    setResizeLimits (500, 340, 1200, 800);
    setSize (640, 460);
}

void PPGWave3Editor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff151515));

    auto top = getLocalBounds().removeFromTop (22);
    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRect (top);
    g.setColour (juce::Colour (0xffffaa00));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("PPG WAVE 3 CLONE   /   PHASE 3",
                top.reduced (10, 0), juce::Justification::centredLeft);

    drawSection (g, osc1Area,    "OSCILLATOR 1");
    drawSection (g, osc2Area,    "OSCILLATOR 2");
    drawSection (g, filterArea,  "FILTER");
    drawSection (g, ampEnvArea,  "AMP ENV");
    drawSection (g, filtEnvArea, "FILTER ENV");
    drawSection (g, masterArea,  "MASTER");
}

void PPGWave3Editor::drawSection (juce::Graphics& g, juce::Rectangle<int> area,
                                  const juce::String& title) const
{
    if (area.isEmpty()) return;
    g.setColour (juce::Colour (0xff2a2a2a));
    g.drawRoundedRectangle (area.toFloat().reduced (0.5f), 4.0f, 1.0f);
    g.setColour (juce::Colour (0xffffaa00));
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText (title, area.getX() + 8, area.getY() + 3, 200, 12,
                juce::Justification::centredLeft);
}

void PPGWave3Editor::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop (26);
    r.reduce (6, 6);

    const int h = r.getHeight();
    const int gap = 5;
    const int oscH  = (int) ((h - 3 * gap) * 0.21f);
    const int filtH = (int) ((h - 3 * gap) * 0.21f);
    const int envH  = h - 2 * oscH - filtH - 3 * gap;

    osc1Area = r.removeFromTop (oscH);
    r.removeFromTop (gap);
    osc2Area = r.removeFromTop (oscH);
    r.removeFromTop (gap);
    filterArea = r.removeFromTop (filtH);
    r.removeFromTop (gap);

    auto bottomRow = r.removeFromTop (envH);
    const int totalW = bottomRow.getWidth();
    const int masterW = (int) (totalW * 0.14f);
    const int envW = (totalW - masterW - 2 * gap) / 2;
    ampEnvArea  = bottomRow.removeFromLeft (envW);
    bottomRow.removeFromLeft (gap);
    filtEnvArea = bottomRow.removeFromLeft (envW);
    bottomRow.removeFromLeft (gap);
    masterArea  = bottomRow;

    // --- Sección de oscilador: info display + selector proporcional + 5 perillas ---
    auto layoutOscSection = [] (juce::Rectangle<int> area,
                                InfoDisplay& info,
                                ButtonSelector& waveSel,
                                RotaryKnob& kPos, RotaryKnob& kOct, RotaryKnob& kSemi,
                                RotaryKnob& kFine, RotaryKnob& kLevel)
    {
        auto inner = area.reduced (8);

        // Fila superior: info display alineado a la derecha
        auto titleRow = inner.removeFromTop (15);
        info.setBounds (titleRow.removeFromRight (155).reduced (0, 1));

        inner.removeFromTop (1);

        // Selector de onda: 30% del ancho disponible (ahora escala horizontalmente)
        const int selW = (int) (inner.getWidth() * 0.30f);
        auto selZone = inner.removeFromLeft (selW).reduced (2, 3);
        waveSel.setBounds (selZone);

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

    // --- Filtro: info display + 3 botones tipo + 4 perillas ---
    {
        auto inner = filterArea.reduced (8);
        auto titleRow = inner.removeFromTop (15);
        filterInfo.setBounds (titleRow.removeFromRight (155).reduced (0, 1));

        inner.removeFromTop (1);

        const int selW = (int) (inner.getWidth() * 0.22f);
        auto selZone = inner.removeFromLeft (selW).reduced (2, 3);
        filterType->setBounds (selZone);

        inner.removeFromLeft (4);
        const int kw = inner.getWidth() / 4;
        filterCutoff  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filterReso    .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filterEnvAmt  .setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filterKeyTrack.setBounds (inner.reduced (1, 0));
    }

    // --- AMP ENV ---
    {
        auto inner = ampEnvArea.reduced (8);
        auto titleRow = inner.removeFromTop (15);
        ampEnvInfo.setBounds (titleRow.removeFromRight (155).reduced (0, 1));

        inner.removeFromTop (1);
        const int kw = inner.getWidth() / 4;
        ampA.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        ampD.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        ampS.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        ampR.setBounds (inner.reduced (1, 0));
    }

    // --- FILTER ENV ---
    {
        auto inner = filtEnvArea.reduced (8);
        auto titleRow = inner.removeFromTop (15);
        filtEnvInfo.setBounds (titleRow.removeFromRight (155).reduced (0, 1));

        inner.removeFromTop (1);
        const int kw = inner.getWidth() / 4;
        filtA.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filtD.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filtS.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        filtR.setBounds (inner.reduced (1, 0));
    }

    // --- MASTER ---
    {
        auto inner = masterArea.reduced (8);
        auto titleRow = inner.removeFromTop (15);
        masterInfo.setBounds (titleRow.reduced (0, 1));

        inner.removeFromTop (1);
        master.setBounds (inner.reduced (2, 0));
    }
}
