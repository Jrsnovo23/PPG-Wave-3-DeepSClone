#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/Oscillator.h"
#include "../DSP/Wavetable.h"
#include "../DSP/Filter.h"
#include "../DSP/LFO.h"

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
        void pitchWheelMoved (int newValue) override;
        void controllerMoved (int controllerNumber, int newValue) override;
        void renderNextBlock (juce::AudioBuffer<float>&, int, int) override;
        void setCurrentPlaybackSampleRate (double newRate) override;

    private:
        // Cada slot de la matriz de modulación: fuente -> destino con cantidad -1..1
        struct ModSlot
        {
            int   source = 0;   // 0=Off, 1=LFO1, 2=LFO2, 3=EnvAmp, 4=EnvFilt,
                                // 5=Velocity, 6=KeyTrack, 7=ModWheel, 8=Aftertouch
            int   dest   = 0;   // 0=Off, 1=PitchOsc1, 2=PitchOsc2, 3=WTPos1,
                                // 4=WTPos2, 5=Cutoff, 6=Reso, 7=Amp, 8=Pan
            float amount = 0.0f;
        };

        juce::AudioProcessorValueTreeState& apvts;

        dsp::Oscillator osc1, osc2;
        dsp::Wavetable  wave1, wave2;
        int currentWave1Idx = -1;
        int currentWave2Idx = -1;

        dsp::Filter filter;
        dsp::LFO    lfo1, lfo2;

        float currentFreq      = 440.0f;
        float currentMidiNote  = 60.0f;
        float velocityGain     = 1.0f;
        float pitchBendValue   = 0.0f;   // -1..1
        float modWheelValue    = 0.0f;   // 0..1
        float aftertouchValue  = 0.0f;   // 0..1
        bool  isActive         = false;

        juce::ADSR adsr;      // Amp envelope
        juce::ADSR filtAdsr;  // Filter envelope

        juce::ADSR::Parameters adsrParams     { 0.005f, 0.100f, 0.700f, 0.300f };
        juce::ADSR::Parameters filtAdsrParams { 0.005f, 0.200f, 0.500f, 0.300f };
    };
}
