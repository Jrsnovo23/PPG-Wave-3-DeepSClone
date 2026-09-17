#include "PluginEditor.h"
#include "ParameterIDs.h"

// ==================== RotaryKnob ====================

PPGWave3Editor::RotaryKnob::RotaryKnob (juce::AudioProcessorValueTreeState& state,
                                        const juce::String& paramID,
                                        const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 58, 13);
    slider.setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (0xffffaa00));
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff333333));
    slider.setColour (juce::Slider::thumbColourId,               juce::Colour (0xffffcc55));
    slider.setColour (juce::Slider::textBoxTextColourId,         juce::Colours::white);
    slider.setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
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
    slider.setBounds (r);
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
      osc1Pos   (p.apvts, ParamIDs::osc1Pos,    "POS"),
      osc1Oct   (p.apvts, ParamIDs::osc1Octave, "OCT"),
      osc1Semi  (p.apvts, ParamIDs::osc1Semi,   "SEMI"),
      osc1Fine  (p.apvts, ParamIDs::osc1Fine,   "FINE"),
      osc1Level (p.apvts, ParamIDs::osc1Level,  "LEVEL"),
      osc2Wave (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::osc2Wave,
                  juce::StringArray { "SIN", "TRI", "SAW", "SQR" })),
      osc2Pos   (p.apvts, ParamIDs::osc2Pos,    "POS"),
      osc2Oct   (p.apvts, ParamIDs::osc2Octave, "OCT"),
      osc2Semi  (p.apvts, ParamIDs::osc2Semi,   "SEMI"),
      osc2Fine  (p.apvts, ParamIDs::osc2Fine,   "FINE"),
      osc2Level (p.apvts, ParamIDs::osc2Level,  "LEVEL"),
      filterType (std::make_unique<ButtonSelector> (p.apvts, ParamIDs::filterType,
                    juce::StringArray { "LP", "HP", "BP" })),
      filterCutoff  (p.apvts, ParamIDs::filterCutoff,   "CUTOFF"),
      filterReso    (p.apvts, ParamIDs::filterReso,     "RESO"),
      filterEnvAmt  (p.apvts, ParamIDs::filterEnvAmt,   "ENV AMT"),
      filterKeyTrack(p.apvts, ParamIDs::filterKeyTrack, "KEY TRK"),
      ampA (p.apvts, ParamIDs::ampAttack,  "A"),
      ampD (p.apvts, ParamIDs::ampDecay,   "D"),
      ampS (p.apvts, ParamIDs::ampSustain, "S"),
      ampR (p.apvts, ParamIDs::ampRelease, "R"),
      filtA (p.apvts, ParamIDs::filtAttack,  "A"),
      filtD (p.apvts, ParamIDs::filtDecay,   "D"),
      filtS (p.apvts, ParamIDs::filtSustain, "S"),
      filtR (p.apvts, ParamIDs::filtRelease, "R"),
      master (p.apvts, ParamIDs::masterGain, "MASTER")
{
    juce::ignoreUnused (processorRef, apvts);

    addAndMakeVisible (*osc1Wave);
    addAndMakeVisible (*osc2Wave);
    addAndMakeVisible (*filterType);

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

    // Reparto vertical: 4 filas
    const int h = r.getHeight();
    const int gap = 5;
    const int oscH  = (int) ((h - 3 * gap) * 0.21f);   // 21% para cada osc
    const int filtH = (int) ((h - 3 * gap) * 0.21f);   // 21% para el filtro
    const int envH  = h - 2 * oscH - filtH - 3 * gap;  // resto para las envolventes

    osc1Area = r.removeFromTop (oscH);
    r.removeFromTop (gap);
    osc2Area = r.removeFromTop (oscH);
    r.removeFromTop (gap);
    filterArea = r.removeFromTop (filtH);
    r.removeFromTop (gap);

    // Fila inferior: AMP ENV + FILTER ENV + MASTER
    auto bottomRow = r.removeFromTop (envH);
    const int totalW = bottomRow.getWidth();
    const int masterW = (int) (totalW * 0.14f);
    const int envW = (totalW - masterW - 2 * gap) / 2;
    ampEnvArea  = bottomRow.removeFromLeft (envW);
    bottomRow.removeFromLeft (gap);
    filtEnvArea = bottomRow.removeFromLeft (envW);
    bottomRow.removeFromLeft (gap);
    masterArea  = bottomRow;

    // --- Helper para colocar el selector de ondas + 5 perillas ---
    auto layoutOscSection = [] (juce::Rectangle<int> area,
                                ButtonSelector& waveSel,
                                RotaryKnob& kPos, RotaryKnob& kOct, RotaryKnob& kSemi,
                                RotaryKnob& kFine, RotaryKnob& kLevel)
    {
        auto inner = area.reduced (8);
        inner.removeFromTop (14);

        auto selZone = inner.removeFromLeft (170).reduced (3, 8);
        waveSel.setBounds (selZone);

        inner.removeFromLeft (4);
        const int kw = inner.getWidth() / 5;
        kPos  .setBounds (inner.removeFromLeft (kw).reduced (1));
        kOct  .setBounds (inner.removeFromLeft (kw).reduced (1));
        kSemi .setBounds (inner.removeFromLeft (kw).reduced (1));
        kFine .setBounds (inner.removeFromLeft (kw).reduced (1));
        kLevel.setBounds (inner.reduced (1));
    };

    layoutOscSection (osc1Area, *osc1Wave, osc1Pos, osc1Oct, osc1Semi, osc1Fine, osc1Level);
    layoutOscSection (osc2Area, *osc2Wave, osc2Pos, osc2Oct, osc2Semi, osc2Fine, osc2Level);

    // --- Filtro: 3 botones tipo + 4 perillas ---
    {
        auto inner = filterArea.reduced (8);
        inner.removeFromTop (14);

        auto selZone = inner.removeFromLeft (130).reduced (3, 8);
        filterType->setBounds (selZone);

        inner.removeFromLeft (4);
        const int kw = inner.getWidth() / 4;
        filterCutoff  .setBounds (inner.removeFromLeft (kw).reduced (1));
        filterReso    .setBounds (inner.removeFromLeft (kw).reduced (1));
        filterEnvAmt  .setBounds (inner.removeFromLeft (kw).reduced (1));
        filterKeyTrack.setBounds (inner.reduced (1));
    }

    // --- AMP ENV ---
    {
        auto inner = ampEnvArea.reduced (8);
        inner.removeFromTop (14);
        const int kw = inner.getWidth() / 4;
        ampA.setBounds (inner.removeFromLeft (kw).reduced (1));
        ampD.setBounds (inner.removeFromLeft (kw).reduced (1));
        ampS.setBounds (inner.removeFromLeft (kw).reduced (1));
        ampR.setBounds (inner.reduced (1));
    }

    // --- FILTER ENV ---
    {
        auto inner = filtEnvArea.reduced (8);
        inner.removeFromTop (14);
        const int kw = inner.getWidth() / 4;
        filtA.setBounds (inner.removeFromLeft (kw).reduced (1));
        filtD.setBounds (inner.removeFromLeft (kw).reduced (1));
        filtS.setBounds (inner.removeFromLeft (kw).reduced (1));
        filtR.setBounds (inner.reduced (1));
    }

    // --- MASTER ---
    {
        auto inner = masterArea.reduced (8);
        inner.removeFromTop (14);
        master.setBounds (inner.reduced (2));
    }
}
