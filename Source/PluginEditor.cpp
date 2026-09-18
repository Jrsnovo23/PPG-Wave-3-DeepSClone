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
      lfo2Wave (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::lfo2Wave, "WAVE", &lfoInfo)),
      lfo2Rate  (p.apvts, ParamIDs::lfo2Rate,  "RATE",  &lfoInfo),
      lfo2Depth (p.apvts, ParamIDs::lfo2Depth, "DEPTH", &lfoInfo),
      mod1Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod1Source, "SRC", &modInfo)),
      mod1Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod1Dest,   "DST", &modInfo)),
      mod1Amt (p.apvts, ParamIDs::mod1Amount, "AMT", &modInfo),
      mod2Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod2Source, "SRC", &modInfo)),
      mod2Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod2Dest,   "DST", &modInfo)),
      mod2Amt (p.apvts, ParamIDs::mod2Amount, "AMT", &modInfo),
      mod3Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod3Source, "SRC", &modInfo)),
      mod3Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod3Dest,   "DST", &modInfo)),
      mod3Amt (p.apvts, ParamIDs::mod3Amount, "AMT", &modInfo),
      mod4Src (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod4Source, "SRC", &modInfo)),
      mod4Dst (std::make_unique<ComboBoxSelector> (p.apvts, ParamIDs::mod4Dest,   "DST", &modInfo)),
      mod4Amt (p.apvts, ParamIDs::mod4Amount, "AMT", &modInfo)
{
    juce::ignoreUnused (processorRef, apvts);

    // --- Selectores (listados uno por uno para evitar ambigüedad de tipos) ---
    addAndMakeVisible (*osc1Wave);
    addAndMakeVisible (*osc2Wave);
    addAndMakeVisible (*filterType);
    addAndMakeVisible (*lfo1Wave);
    addAndMakeVisible (*lfo2Wave);
    addAndMakeVisible (*mod1Src);
    addAndMakeVisible (*mod1Dst);
    addAndMakeVisible (*mod2Src);
    addAndMakeVisible (*mod2Dst);
    addAndMakeVisible (*mod3Src);
    addAndMakeVisible (*mod3Dst);
    addAndMakeVisible (*mod4Src);
    addAndMakeVisible (*mod4Dst);

    // --- Info displays ---
    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &ampEnvInfo, &filtEnvInfo, &masterInfo,
                     &lfoInfo, &modInfo })
        addAndMakeVisible (d);

    // --- Perillas ---
    std::initializer_list<juce::Component*> allKnobs {
        &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
        &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
        &filterCutoff, &filterReso, &filterEnvAmt, &filterKeyTrack,
        &ampA, &ampD, &ampS, &ampR,
        &filtA, &filtD, &filtS, &filtR,
        &master,
        &lfo1Rate, &lfo1Depth, &lfo2Rate, &lfo2Depth,
        &mod1Amt, &mod2Amt, &mod3Amt, &mod4Amt
    };
    for (auto* c : allKnobs)
        addAndMakeVisible (c);

    setResizable (true, true);
    setResizeLimits (700, 620, 1400, 1260);
    setSize (860, 720);
}

float PPGWave3Editor::computeScale() const
{
    const float refH = 720.0f;
    return juce::jlimit (0.72f, 1.5f, (float) getHeight() / refH);
}

void PPGWave3Editor::applyScaleToAll (float scaleValue)
{
    // Perillas
    for (auto* k : { &osc1Pos, &osc1Oct, &osc1Semi, &osc1Fine, &osc1Level,
                     &osc2Pos, &osc2Oct, &osc2Semi, &osc2Fine, &osc2Level,
                     &filterCutoff, &filterReso, &filterEnvAmt, &filterKeyTrack,
                     &ampA, &ampD, &ampS, &ampR,
                     &filtA, &filtD, &filtS, &filtR,
                     &master,
                     &lfo1Rate, &lfo1Depth, &lfo2Rate, &lfo2Depth,
                     &mod1Amt, &mod2Amt, &mod3Amt, &mod4Amt })
        k->setScale (scaleValue);

    // Info displays
    for (auto* d : { &osc1Info, &osc2Info, &filterInfo,
                     &ampEnvInfo, &filtEnvInfo, &masterInfo,
                     &lfoInfo, &modInfo })
        d->setScale (scaleValue);

    // ButtonSelectors (todos del mismo tipo)
    osc1Wave  ->setScale (scaleValue);
    osc2Wave  ->setScale (scaleValue);
    filterType->setScale (scaleValue);

    // ComboBoxSelectors (todos del mismo tipo)
    lfo1Wave->setScale (scaleValue);
    lfo2Wave->setScale (scaleValue);
    mod1Src ->setScale (scaleValue);
    mod1Dst ->setScale (scaleValue);
    mod2Src ->setScale (scaleValue);
    mod2Dst ->setScale (scaleValue);
    mod3Src ->setScale (scaleValue);
    mod3Dst ->setScale (scaleValue);
    mod4Src ->setScale (scaleValue);
    mod4Dst ->setScale (scaleValue);
}

void PPGWave3Editor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff151515));

    auto top = getLocalBounds().removeFromTop (24);
    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRect (top);
    g.setColour (juce::Colour (0xffffaa00));
    g.setFont (juce::FontOptions (12.0f * currentScale, juce::Font::bold));
    g.drawText ("PPG WAVE 3 CLONE   /   PHASE 4",
                top.reduced (10, 0), juce::Justification::centredLeft);

    drawSection (g, osc1Area,    "OSCILLATOR 1");
    drawSection (g, osc2Area,    "OSCILLATOR 2");
    drawSection (g, filterArea,  "FILTER");
    drawSection (g, lfoArea,     "LFO");
    drawSection (g, modArea,     "MOD MATRIX");
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
    g.setFont (juce::FontOptions (10.0f * currentScale, juce::Font::bold));
    g.drawText (title, area.getX() + 8, area.getY() + 3, 200, 12,
                juce::Justification::centredLeft);
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
    const int availH = h - 5 * gap;

    const float availHf = (float) availH;
    const int oscH  = (int) (availHf * 0.1407f);
    const int filtH = (int) (availHf * 0.1407f);
    const int lfoH  = (int) (availHf * 0.1529f);
    const int modH  = (int) (availHf * 0.2370f);
    const int envH  = availH - 2 * oscH - filtH - lfoH - modH;

    osc1Area = r.removeFromTop (oscH);          r.removeFromTop (gap);
    osc2Area = r.removeFromTop (oscH);          r.removeFromTop (gap);
    filterArea = r.removeFromTop (filtH);       r.removeFromTop (gap);
    lfoArea = r.removeFromTop (lfoH);           r.removeFromTop (gap);
    modArea = r.removeFromTop (modH);           r.removeFromTop (gap);

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

    // -------- Osciladores --------
    auto layoutOscSection = [&] (juce::Rectangle<int> area,
                                 InfoDisplay& info, ButtonSelector& waveSel,
                                 RotaryKnob& kPos, RotaryKnob& kOct, RotaryKnob& kSemi,
                                 RotaryKnob& kFine, RotaryKnob& kLevel)
    {
        auto inner = area.reduced (8);
        auto titleRow = inner.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        info.setBounds (titleRow.removeFromRight (
            infoWidthFor (area.getWidth())).reduced (0, 1));
        inner.removeFromTop (1);

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
        auto inner = filterArea.reduced (8);
        auto titleRow = inner.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        filterInfo.setBounds (titleRow.removeFromRight (
            infoWidthFor (filterArea.getWidth())).reduced (0, 1));
        inner.removeFromTop (1);

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
        auto inner = lfoArea.reduced (8);
        auto titleRow = inner.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        lfoInfo.setBounds (titleRow.removeFromRight (
            infoWidthFor (lfoArea.getWidth())).reduced (0, 1));
        inner.removeFromTop (1);

        const int halfW = (inner.getWidth() - gap) / 2;

        auto layoutLFO = [&] (juce::Rectangle<int> area,
                              ComboBoxSelector& w, RotaryKnob& rate, RotaryKnob& depth)
        {
            const int comboW = (int) ((float) area.getWidth() * 0.32f);
            w.setBounds (area.removeFromLeft (comboW).reduced (2, 0));
            area.removeFromLeft (2);
            const int kw = area.getWidth() / 2;
            rate .setBounds (area.removeFromLeft (kw).reduced (1, 0));
            depth.setBounds (area.reduced (1, 0));
        };

        auto lfo1Zone = inner.removeFromLeft (halfW);
        inner.removeFromLeft (gap);
        auto lfo2Zone = inner;

        layoutLFO (lfo1Zone, *lfo1Wave, lfo1Rate, lfo1Depth);
        layoutLFO (lfo2Zone, *lfo2Wave, lfo2Rate, lfo2Depth);
    }

    // -------- Mod Matrix --------
    {
        auto inner = modArea.reduced (8);
        auto titleRow = inner.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        modInfo.setBounds (titleRow.removeFromRight (
            infoWidthFor (modArea.getWidth())).reduced (0, 1));
        inner.removeFromTop (1);

        const int rowH = inner.getHeight() / 4;

        auto layoutRow = [&] (juce::Rectangle<int> row,
                              ComboBoxSelector& src, ComboBoxSelector& dst, RotaryKnob& amt)
        {
            const int srcW = (int) ((float) row.getWidth() * 0.40f);
            const int dstW = (int) ((float) row.getWidth() * 0.40f);
            src.setBounds (row.removeFromLeft (srcW).reduced (2, 1));
            dst.setBounds (row.removeFromLeft (dstW).reduced (2, 1));
            amt.setBounds (row.reduced (2, 1));
        };

        layoutRow (inner.removeFromTop (rowH), *mod1Src, *mod1Dst, mod1Amt);
        layoutRow (inner.removeFromTop (rowH), *mod2Src, *mod2Dst, mod2Amt);
        layoutRow (inner.removeFromTop (rowH), *mod3Src, *mod3Dst, mod3Amt);
        layoutRow (inner,                        *mod4Src, *mod4Dst, mod4Amt);
    }

    // -------- Amp Env --------
    {
        auto inner = ampEnvArea.reduced (8);
        auto titleRow = inner.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        ampEnvInfo.setBounds (titleRow.removeFromRight (
            infoWidthFor (ampEnvArea.getWidth())).reduced (0, 1));
        inner.removeFromTop (1);
        const int kw = inner.getWidth() / 4;
        ampA.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        ampD.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        ampS.setBounds (inner.removeFromLeft (kw).reduced (1, 0));
        ampR.setBounds (inner.reduced (1, 0));
    }

    // -------- Filter Env --------
    {
        auto inner = filtEnvArea.reduced (8);
        auto titleRow = inner.removeFromTop (
            juce::jmax (14, juce::roundToInt (15.0f * currentScale)));
        filtEnvInfo.setBounds (titleRow.removeFromRight (
            infoWidthFor (filtEnvArea.getWidth())).reduced (0, 1));
        inner.removeFromTop (1);
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
