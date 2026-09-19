#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>
#include "../DSP/Wavetable.h"

namespace ui
{
    // ============================================================
    // WavetablePreview — dibuja la forma de onda actual del osc.
    // ============================================================
    class WavetablePreview : public juce::Component
    {
    public:
        WavetablePreview (juce::AudioProcessorValueTreeState& apvts,
                          const juce::String& waveParamID,
                          const juce::String& posParamID);
        void paint (juce::Graphics&) override;
    private:
        void refreshIfNeeded();
        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::String waveId, posId;
        int   cachedWave = -1;
        float cachedPos  = -1.0f;
        dsp::Wavetable cachedTable;
    };

    // ============================================================
    // EnvelopeDisplay — dibuja la curva ADSR.
    // ============================================================
    class EnvelopeDisplay : public juce::Component
    {
    public:
        EnvelopeDisplay (juce::AudioProcessorValueTreeState& apvts,
                         const juce::String& aId, const juce::String& dId,
                         const juce::String& sId, const juce::String& rId);
        void paint (juce::Graphics&) override;
    private:
        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::String attackId, decayId, sustainId, releaseId;
    };

    // ============================================================
    // LFODisplay — dibuja la forma de onda del LFO (un ciclo).
    // ============================================================
    class LFODisplay : public juce::Component
    {
    public:
        LFODisplay (juce::AudioProcessorValueTreeState& apvts,
                    const juce::String& waveParamID);
        void paint (juce::Graphics&) override;
    private:
        juce::AudioProcessorValueTreeState& apvtsRef;
        juce::String waveId;
    };

    // ============================================================
    // LevelMeter — barra horizontal que muestra el nivel de salida.
    // ============================================================
    class LevelMeter : public juce::Component, private juce::Timer
    {
    public:
        explicit LevelMeter (std::atomic<float>& levelSource);
        void paint (juce::Graphics&) override;
    private:
        void timerCallback() override;
        std::atomic<float>& level;
        float smoothed = 0.0f;
    };
}
