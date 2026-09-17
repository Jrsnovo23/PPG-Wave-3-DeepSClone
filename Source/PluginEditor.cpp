#include "PluginEditor.h"
#include "ParameterIDs.h"

// ==================== RotaryKnob ====================

PPGWave3Editor::RotaryKnob::RotaryKnob (juce::AudioProcessorValueTreeState& state,
                                        const juce::String& paramID,
                                        const juce::String& labelText)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 16);
    slider.setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (0xffffaa00));
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff333333));
    slider.setColour (juce::Slider::thumbColourId,               juce::Colour (0xffffcc55));
    slider.setColour (juce::Slider::textBoxTextColourId,         juce::Colours::white);
    slider.setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffcccccc));
    label.setFont (juce::FontOptions (11.0f));
    addAndMakeVisible (label);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        state, paramID, slider);
}

void PPGWave3Editor::RotaryKnob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (14));
    slider.setBounds (r);
}

void PPGWave3Editor::RotaryKnob::paint (juce::Graphics&) {}

// ==================== WaveSelector ====================

static const char* kWaveNames[4] = { "SIN", "TRI", "SAW", "SQR" };

PPGWave3Editor::WaveSelector::WaveSelector (juce::AudioProcessorValueTreeState& state,
                                            const juce::String& paramID)
    : apvtsRef (state), id (paramID)
{
    for (int i = 0; i < 4; ++i)
    {
        auto* b = buttons.add (new juce::TextButton (kWaveNames[i]));
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

void PPGWave3Editor::WaveSelector::refreshFromParameter()
{
    for (int i = 0; i < buttons.size(); ++i)
        buttons[i]->setToggleState (i == currentIndex, juce::dontSendNotification);
}

void PPGWave3Editor::WaveSelector::resized()
{
    auto r = getLocalBounds();
    const int w = r.getWidth() / juce::jmax (1, buttons.size());
    for (auto* b : buttons)
        b->setBounds (r.removeFromLeft (w).reduced (2));
}

void PPGWave3Editor::WaveSelector::paint (juce::Graphics&) {}

// ==================== Editor ====================

PPGWave3Editor::PPGWave3Editor (PPGWave3Processor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      apvts (p.apvts),
      osc1Wave (std::make_unique<WaveSelector> (p.apvts, ParamIDs::osc1Wave)),
      osc1Pos   (p.apvts, ParamIDs::osc1Pos,    "POS"),
      osc1Oct   (p.apvts, ParamIDs::osc1Octave, "OCT"),
      osc1Semi  (p.apvts, ParamIDs::osc1Semi,   "SEMI"),
      osc1Fine  (p.apvts, ParamIDs::osc1Fine,   "FINE"),
      osc1Level (p.apvts, ParamIDs::osc1Level,  "LEVEL"),
      osc2Wave (std::make_unique<WaveSelector> (p.apvts, ParamIDs::osc2Wave)),
      osc2Pos   (p.apvts, ParamIDs::osc2Pos,    "POS"),
      osc2Oct   (p.apvts, ParamIDs::osc2Octave, "OCT"),
      osc2Semi  (p.apvts, ParamIDs::osc2Semi,   "SEMI"),
      osc2Fine  (p.apvts, ParamIDs::osc2Fine,   "FINE"),
      osc2Level (p.apvts, ParamIDs::osc2Level,  "LEVEL"),
      ampA (p.apvts, ParamIDs::ampAttack,  "ATTACK"),
      ampD (p.apvts, ParamIDs::ampDecay,   "DECAY"),
      ampS (p.apvts, ParamIDs::ampSustain, "SUSTAIN"),
      ampR (p.apvts, ParamIDs::ampRelease, "RELEASE"),
      master (p.apvts, ParamIDs::masterGain, "MASTER")
{
    juce::ignoreUnused (processorRef, apvts);

    addAndMakeVisible (*osc1Wave);
    addAndMakeVisible (*osc2Wave);

    std::initializer_list<juce::Component*> allKnobs {
        &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
        &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
        &ampA, &ampD, &ampS, &ampR,
        &master
    };
    for (auto* c : allKnobs)
        addAndMakeVisible (c);

    setResizable (true, true);
    setResizeLimits (700, 460, 1400, 900);
    setSize (760, 500);
}

void PPGWave3Editor::paint (juce::Graphics& g)
{
    // Fondo general
    g.fillAll (juce::Colour (0xff151515));

    // Barra superior
    auto top = getLocalBounds().removeFromTop (28);
    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRect (top);
    g.setColour (juce::Colour (0xffffaa00));
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("PPG WAVE 3 CLONE   /   PHASE 2",
                top.reduced (12, 0), juce::Justification::centredLeft);

    // Marcos de sección
    drawSection (g, osc1Area, "OSCILLATOR 1");
    drawSection (g, osc2Area, "OSCILLATOR 2");
    drawSection (g, envArea,  "AMP ENVELOPE + MASTER");
}

void PPGWave3Editor::drawSection (juce::Graphics& g, juce::Rectangle<int> area,
                                  const juce::String& title) const
{
    if (area.isEmpty())
        return;

    g.setColour (juce::Colour (0xff2a2a2a));
    g.drawRoundedRectangle (area.toFloat().reduced (0.5f), 4.0f, 1.0f);
    g.setColour (juce::Colour (0xffffaa00));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText (title, area.getX() + 10, area.getY() + 4, 220, 14,
                juce::Justification::centredLeft);
}

void PPGWave3Editor::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop (34);                       // barra superior
    r.reduce (8, 8);

    // Reparto vertical: 3 secciones
    const int sectionH = (r.getHeight() - 16) / 3;

    osc1Area = r.removeFromTop (sectionH);
    r.removeFromTop (8);
    osc2Area = r.removeFromTop (sectionH);
    r.removeFromTop (8);
    envArea  = r;

    // --- Contenido de cada sección ---
    auto layoutSection = [] (juce::Rectangle<int> area,
                             WaveSelector& waveSel,
                             RotaryKnob& kPos, RotaryKnob& kOct, RotaryKnob& kSemi,
                             RotaryKnob& kFine, RotaryKnob& kLevel)
    {
        auto inner = area.reduced (10);
        inner.removeFromTop (16);               // título

        // Zona izquierda: selector de onda (4 botones en fila)
        auto selZone = inner.removeFromLeft (220).reduced (4, 14);
        waveSel.setBounds (selZone);

        // Zona derecha: 5 perillas
        inner.removeFromLeft (6);
        const int kw = inner.getWidth() / 5;
        kPos  .setBounds (inner.removeFromLeft (kw).reduced (2));
        kOct  .setBounds (inner.removeFromLeft (kw).reduced (2));
        kSemi .setBounds (inner.removeFromLeft (kw).reduced (2));
        kFine .setBounds (inner.removeFromLeft (kw).reduced (2));
        kLevel.setBounds (inner.reduced (2));
    };

    layoutSection (osc1Area, *osc1Wave, osc1Pos, osc1Oct, osc1Semi, osc1Fine, osc1Level);
    layoutSection (osc2Area, *osc2Wave, osc2Pos, osc2Oct, osc2Semi, osc2Fine, osc2Level);

    // --- Sección de envolvente + master ---
    {
        auto inner = envArea.reduced (10);
        inner.removeFromTop (16);
        const int kw = inner.getWidth() / 5;
        ampA  .setBounds (inner.removeFromLeft (kw).reduced (2));
        ampD  .setBounds (inner.removeFromLeft (kw).reduced (2));
        ampS  .setBounds (inner.removeFromLeft (kw).reduced (2));
        ampR  .setBounds (inner.removeFromLeft (kw).reduced (2));
        master.setBounds (inner.reduced (2));
    }
}
