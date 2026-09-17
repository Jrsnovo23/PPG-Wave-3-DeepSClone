#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/Oscillator.h"
#include "../DSP/Wavetable.h"

namespace synth
{
    class SynthVoice : public juce::SynthesiserVoice
    {
    public:
        explicit SynthVoice (juce::AudioProcessorValueTreeState& apvts);

        bool canPlaySound (juce::SynthesiserSound*) override;
        void startNote (int midiNote, float velocity,
                        juce::SynthesiserSound*, int currentPitchWheelPos) override;
        void stopNote (float velocity, bool allowTailOff) override;
        void pitchWheelMoved (int) override {}
        void controllerMoved (int, int) override {}
        void renderNextBlock (juce::AudioBuffer<float>&, int, int) override;
        void setCurrentPlaybackSampleRate (double newRate) override;

    private:
        juce::AudioProcessorValueTreeState& apvts;

        dsp::Oscillator osc1, osc2;
        dsp::Wavetable  wave1, wave2;
        int currentWave1Idx = -1;
        int currentWave2Idx = -1;

        float currentFreq = 440.0f;
        float velocityGain = 1.0f;
        bool  isActive = false;

        juce::ADSR adsr;
        juce::ADSR::Parameters adsrParams { 0.005f, 0.1f, 0.7f, 0.2f };
    };
}