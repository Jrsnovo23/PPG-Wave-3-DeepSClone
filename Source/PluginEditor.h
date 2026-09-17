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
    // --- Ventana de información por sección (muestra último parámetro tocado) ---
    class InfoDisplay : public juce::Component
    {
    public:
        void setInfo (const juce::String& name, const juce::String& value);
        void paint (juce::Graphics&) override;

    private:
        juce::String paramName { "--" };
        juce::String paramValue;
    };

    // --- Perilla giratoria con etiqueta ---
    class RotaryKnob : public juce::Component
    {
    public:
        RotaryKnob (juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& paramID,
                    const juce::String& labelText,
                    InfoDisplay* display);
        void resized() override;
        void paint   (juce::Graphics&) override;

    private:
        juce::Slider  slider;
        juce::Label   label;
        InfoDisplay*  infoDisplay = nullptr;
        juce::String  paramName;
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

    // Info displays (uno por sección)
    InfoDisplay osc1Info, osc2Info, filterInfo, ampEnvInfo, filtEnvInfo, masterInfo;

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

    juce::Rectangle<int> osc1Area, osc2Area, filterArea, ampEnvArea, filtEnvArea, masterArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PPGWave3Editor)
};
