#include "PluginEditor.h"

PPGWave3Editor::PPGWave3Editor (PPGWave3Processor& p)
    : AudioProcessorEditor (&p), processorRef (p), genericUI (p)
{
    addAndMakeVisible (genericUI);
    setSize (900, 620);
}

void PPGWave3Editor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a1a));
    g.setColour (juce::Colour (0xffffaa00));
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("PPG WAVE 3 CLONE  ·  PHASE 1",
                14, 8, getWidth() - 28, 26,
                juce::Justification::centredLeft);
}

void PPGWave3Editor::resized()
{
    genericUI.setBounds (getLocalBounds().withTrimmedTop (40));
}
