#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class PPGWave3Editor : public juce::AudioProcessorEditor
{
public:
    explicit PPGWave3Editor (PPGWave3Processor&);
    ~PPGWave3Editor() override = default;

    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    // --- Perilla giratoria con etiqueta ---
    class RotaryKnob : public juce::Component
    {
    public:
        RotaryKnob (juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& paramID,
                    const juce::String& labelText);
        void resized() override;
        void paint   (juce::Graphics&) override;

    private:
        juce::Slider  slider;
        juce::Label   label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    // --- Selector con N botones (para ondas y para tipo de filtro) ---
    class ButtonSelector : public juce::Component
    {
    public:
        ButtonSelector (juce::AudioProcessorValueTreeState& apvts,
                        const juce::String& paramID,
                        const juce::StringArray& names);
        void resized() override;
        void paint   (juce::Graphics&) override;

    private:
        void refreshFromParameter();

        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::String id;
        juce::OwnedArray<juce::TextButton> buttons;
        std::unique_ptr<juce::ParameterAttachment> attachment;
        int currentIndex = 0;
    };

    void drawSection (juce::Graphics& g, juce::Rectangle<int> area,
                      const juce::String& title) const;

    PPGWave3Processor& processorRef;
    juce::AudioProcessorValueTreeState& apvts;

    // Oscilador 1
    std::unique_ptr<ButtonSelector> osc1Wave;
    RotaryKnob osc1Pos, osc1Oct, osc1Semi, osc1Fine, osc1Level;

    // Oscilador 2
    std::unique_ptr<ButtonSelector> osc2Wave;
    RotaryKnob osc2Pos, osc2Oct, osc2Semi, osc2Fine, osc2Level;

    // Filtro
    std::unique_ptr<ButtonSelector> filterType;
    RotaryKnob filterCutoff, filterReso, filterEnvAmt, filterKeyTrack;

    // Amp Envelope
    RotaryKnob ampA, ampD, ampS, ampR;

    // Filter Envelope
    RotaryKnob filtA, filtD, filtS, filtR;

    // Master
    RotaryKnob master;

    // Áreas reservadas (calculadas en resized)
    juce::Rectangle<int> osc1Area, osc2Area, filterArea, ampEnvArea, filtEnvArea, masterArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PPGWave3Editor)
};
