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
    // ---------- InfoDisplay ----------
    class InfoDisplay : public juce::Component
    {
    public:
        void setInfo (const juce::String& name, const juce::String& value);
        void setScale (float s) { scale = s; repaint(); }
        void paint (juce::Graphics&) override;
    private:
        juce::String paramName { "--" };
        juce::String paramValue;
        float scale = 1.0f;
    };

    // ---------- RotaryKnob ----------
    class RotaryKnob : public juce::Component
    {
    public:
        RotaryKnob (juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& paramID,
                    const juce::String& labelText,
                    InfoDisplay* display);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s);
    private:
        juce::Slider  slider;
        juce::Label   label;
        InfoDisplay*  infoDisplay = nullptr;
        juce::String  paramName;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        float         scale = 1.0f;
    };

    // ---------- ButtonSelector ----------
    class ButtonSelector : public juce::Component
    {
    public:
        ButtonSelector (juce::AudioProcessorValueTreeState& apvts,
                        const juce::String& paramID,
                        const juce::StringArray& names);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s) { scale = s; }
    private:
        void refreshFromParameter();
        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::String id;
        juce::OwnedArray<juce::TextButton> buttons;
        std::unique_ptr<juce::ParameterAttachment> attachment;
        int currentIndex = 0;
        float scale = 1.0f;
    };

    // ---------- ComboBoxSelector ----------
    class ComboBoxSelector : public juce::Component
    {
    public:
        ComboBoxSelector (juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& paramID,
                          const juce::String& labelText,
                          InfoDisplay* display);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s);
    private:
        juce::ComboBox combo;
        juce::Label    label;
        InfoDisplay*   infoDisplay = nullptr;
        juce::String   paramName;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
        float          scale = 1.0f;
    };

    // ---------- Helpers ----------
    void  drawSection (juce::Graphics& g, juce::Rectangle<int> area,
                       const juce::String& title) const;
    float computeScale() const;
    void  applyScaleToAll (float s);

    PPGWave3Processor& processorRef;
    juce::AudioProcessorValueTreeState& apvts;

    // Info displays (uno por sección, ahora 8)
    InfoDisplay osc1Info, osc2Info, filterInfo;
    InfoDisplay ampEnvInfo, filtEnvInfo, masterInfo;
    InfoDisplay lfoInfo, modInfo;

    // Osciladores
    std::unique_ptr<ButtonSelector> osc1Wave;
    RotaryKnob osc1Pos, osc1Oct, osc1Semi, osc1Fine, osc1Level;
    std::unique_ptr<ButtonSelector> osc2Wave;
    RotaryKnob osc2Pos, osc2Oct, osc2Semi, osc2Fine, osc2Level;

    // Filtro
    std::unique_ptr<ButtonSelector> filterType;
    RotaryKnob filterCutoff, filterReso, filterEnvAmt, filterKeyTrack;

    // Amp Env
    RotaryKnob ampA, ampD, ampS, ampR;

    // Filter Env
    RotaryKnob filtA, filtD, filtS, filtR;

    // Master
    RotaryKnob master;

    // LFO 1
    std::unique_ptr<ComboBoxSelector> lfo1Wave;
    RotaryKnob lfo1Rate, lfo1Depth, lfo1Phase;

    // LFO 2
    std::unique_ptr<ComboBoxSelector> lfo2Wave;
    RotaryKnob lfo2Rate, lfo2Depth, lfo2Phase;

    // Mod Matrix — 4 slots
    std::unique_ptr<ComboBoxSelector> mod1Src, mod1Dst;  RotaryKnob mod1Amt;
    std::unique_ptr<ComboBoxSelector> mod2Src, mod2Dst;  RotaryKnob mod2Amt;
    std::unique_ptr<ComboBoxSelector> mod3Src, mod3Dst;  RotaryKnob mod3Amt;
    std::unique_ptr<ComboBoxSelector> mod4Src, mod4Dst;  RotaryKnob mod4Amt;

    // Áreas
    juce::Rectangle<int> osc1Area, osc2Area, filterArea;
    juce::Rectangle<int> lfoArea, modArea;
    juce::Rectangle<int> ampEnvArea, filtEnvArea, masterArea;

    float currentScale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PPGWave3Editor)
};
