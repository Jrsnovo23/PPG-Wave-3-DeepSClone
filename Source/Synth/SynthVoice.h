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
        struct ModSlot
        {
            int   source = 0;   // 0=None, 1=LFO1, 2=LFO2, 3=Env1(Amp), 4=Env2(Filt),
                                // 5=Velocity, 6=ModWheel, 7=Aftertouch,
                                // 8=NoteNumber, 9=Random
            int   dest   = 0;   // 0=None, 1=OSC1Pitch, 2=OSC2Pitch, 3=OSC1WTPos,
                                // 4=OSC2WTPos, 5=Cutoff, 6=Amp
            float amount = 0.0f;
        };

        juce::AudioProcessorValueTreeState& apvts;

        dsp::Oscillator osc1, osc2;
        dsp::Wavetable  wave1, wave2;
        int currentWave1Idx = -1;
        int currentWave2Idx = -1;

        dsp::Filter filter;
        dsp::LFO    lfo1, lfo2;

        float currentFreq     = 440.0f;
        float currentMidiNote = 60.0f;
        float velocityGain    = 1.0f;
        float pitchBendValue  = 0.0f;
        float modWheelValue   = 0.0f;
        float aftertouchValue = 0.0f;
        float randomValue     = 0.0f;
        bool  isActive        = false;

        juce::ADSR adsr;
        juce::ADSR filtAdsr;

        juce::ADSR::Parameters adsrParams     { 0.005f, 0.100f, 0.700f, 0.300f };
        juce::ADSR::Parameters filtAdsrParams { 0.005f, 0.200f, 0.500f, 0.300f };
    };
}
