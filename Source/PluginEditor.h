#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/PPGLookAndFeel.h"
#include "UI/Visualizers.h"
#include "Presets/PresetManager.h"

class PPGWave3Editor : public juce::AudioProcessorEditor
{
public:
    explicit PPGWave3Editor (PPGWave3Processor&);
    ~PPGWave3Editor() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

private:
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

    class HSlider : public juce::Component
    {
    public:
        HSlider (juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& paramID,
                 InfoDisplay* display);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s);
    private:
        juce::Slider  slider;
        InfoDisplay*  infoDisplay = nullptr;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
        float         scale = 1.0f;
    };

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

    class ToggleButton : public juce::Component
    {
    public:
        ToggleButton (juce::AudioProcessorValueTreeState& apvts,
                      const juce::String& paramID,
                      const juce::String& labelText,
                      InfoDisplay* display);
        void resized() override;
        void paint   (juce::Graphics&) override;
        void setScale (float s);
    private:
        juce::TextButton button;
        InfoDisplay*     infoDisplay = nullptr;
        juce::String     paramName;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
        float            scale = 1.0f;
    };

    class PresetDisplay : public juce::Component
    {
    public:
        void setInfo (const juce::String& name, const juce::String& category, bool factory);
        void paint (juce::Graphics&) override;
    private:
        juce::String presetName { "Init Saw" };
        juce::String presetCategory { "Init" };
        bool isFactory = true;
    };

    void  drawSection (juce::Graphics& g, juce::Rectangle<int> area,
                       const juce::String& title) const;
    void  drawLogo (juce::Graphics& g, juce::Rectangle<int> area) const;
    float computeScale() const;
    void  applyScaleToAll (float s);

    void  updatePresetDisplay();
    void  onPrevPreset();
    void  onNextPreset();
    void  onLoadPreset();
    void  onSavePreset();
    void  onBrowsePreset();

    void  updateFxVisibility();
    void  updateEnvVisibility();

    PPGWave3Processor& processorRef;
    juce::AudioProcessorValueTreeState& apvts;
    PPGLookAndFeel ppgLnf;

    presets::PresetManager presetManager;

    // === Preset bar ===
    juce::TextButton prevBtn, nextBtn;
    juce::TextButton loadBtn, saveBtn, browseBtn;
    PresetDisplay    presetDisplay;
    std::unique_ptr<juce::FileChooser> fileChooser;

    InfoDisplay osc1Info, osc2Info, filterInfo;
    InfoDisplay envInfo, masterInfo;
    InfoDisplay lfoInfo, modInfo, fxInfo;

    // Osciladores
    std::unique_ptr<ButtonSelector> osc1Wave;
    ui::WavetablePreview osc1Preview;
    RotaryKnob osc1Pos, osc1Oct, osc1Semi, osc1Fine, osc1Level;
    std::unique_ptr<ButtonSelector> osc2Wave;
    ui::WavetablePreview osc2Preview;
    RotaryKnob osc2Pos, osc2Oct, osc2Semi, osc2Fine, osc2Level;

    // Filtro
    std::unique_ptr<ButtonSelector> filterType;
    RotaryKnob filterCutoff, filterReso, filterEnvAmt, filterKeyTrack;

    // Envelopes con tabs
    ui::EnvelopeDisplay env1Display, env2Display, env3Display;
    RotaryKnob env1A, env1D, env1S, env1R;
    RotaryKnob env2A, env2D, env2S, env2R;
    RotaryKnob env3A, env3D, env3S, env3R;
    juce::TextButton env1TabBtn, env2TabBtn, env3TabBtn;
    int activeEnvTab = 0;

    // Master
    RotaryKnob master;
    ui::LevelMeter masterMeter;

    // LFO 1
    std::unique_ptr<ComboBoxSelector> lfo1Wave;
    ui::LFODisplay lfo1Display;
    RotaryKnob lfo1Rate, lfo1Depth, lfo1Phase;
    std::unique_ptr<ComboBoxSelector> lfo1Sync;

    // LFO 2
    std::unique_ptr<ComboBoxSelector> lfo2Wave;
    ui::LFODisplay lfo2Display;
    RotaryKnob lfo2Rate, lfo2Depth, lfo2Phase;
    std::unique_ptr<ComboBoxSelector> lfo2Sync;

    // Mod Matrix
    std::unique_ptr<ComboBoxSelector> mod1Src, mod1Dst;  HSlider mod1Amt;
    std::unique_ptr<ComboBoxSelector> mod2Src, mod2Dst;  HSlider mod2Amt;
    std::unique_ptr<ComboBoxSelector> mod3Src, mod3Dst;  HSlider mod3Amt;
    std::unique_ptr<ComboBoxSelector> mod4Src, mod4Dst;  HSlider mod4Amt;

    // ===== FX — pestañas (FASE 6.7: 6 tabs) =====
    juce::TextButton driveTabBtn, chorusTabBtn, delayTabBtn, reverbTabBtn;
    juce::TextButton eqTabBtn, phaserTabBtn;
    int activeFxTab = 0;   // 0=Drive, 1=Chorus, 2=Delay, 3=Reverb, 4=EQ, 5=Phaser

    // FX — Drive
    std::unique_ptr<ToggleButton> driveOn;
    RotaryKnob driveAmount, driveTone, driveMix;
    // FX — Chorus
    std::unique_ptr<ToggleButton> chorusOn;
    RotaryKnob chorusRate, chorusDepth, chorusMix;
    // FX — Delay
    std::unique_ptr<ToggleButton> delayOn;
    std::unique_ptr<ComboBoxSelector> delaySync;
    RotaryKnob delayTime, delayFeedback, delayMix;
    // FX — Reverb
    std::unique_ptr<ToggleButton> reverbOn;
    RotaryKnob reverbSize, reverbDamp, reverbMix;

    // ===== FASE 6.7: FX — EQ (4 bandas × Freq+Gain) =====
    std::unique_ptr<ToggleButton> eqOn;
    RotaryKnob eqLowFreq,  eqLowGain;
    RotaryKnob eqLmidFreq, eqLmidGain;
    RotaryKnob eqHmidFreq, eqHmidGain;
    RotaryKnob eqHighFreq, eqHighGain;

    // ===== FASE 6.7: FX — Phaser =====
    std::unique_ptr<ToggleButton> phaserOn;
    RotaryKnob phaserRate, phaserDepth, phaserFeedback, phaserMix;

    // ===== Teclado virtual + Pitch/Mod wheels =====
    juce::MidiKeyboardComponent keyboardComponent;
    juce::Slider pitchWheelSlider;
    juce::Slider modWheelSlider;
    juce::Label  pitchWheelLabel, modWheelLabel;

    juce::Rectangle<int> osc1Area, osc2Area, filterArea;
    juce::Rectangle<int> lfoArea, modArea, fxArea;
    juce::Rectangle<int> envArea, masterArea;
    juce::Rectangle<int> keyboardArea, wheelsArea;
    juce::Rectangle<int> presetBarArea;

    float currentScale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PPGWave3Editor)
};
