#pragma once
#include <juce_dsp/juce_dsp.h>

namespace dsp
{
    // Cadena de efectos globales aplicada post-mix.
    // Orden: EQ -> Drive -> Chorus -> Phaser -> Delay -> Reverb.
    class Effects
    {
    public:
        void prepare (double sampleRate, int maxBlockSize, int numChannels);
        void reset();
        void process (juce::AudioBuffer<float>& buffer);

        // Configuración antes de process(). Todos los "on" son booleanos.
        void setChorus (bool on, float rate, float depth, float mix);
        void setDelay  (bool on, float timeSec, float feedback, float mix);
        void setReverb (bool on, float roomSize, float damping, float mix);
        void setDrive  (bool on, float drive, float tone, float mix);

        // ===== FASE 6.7 =====
        void setEQ     (bool on,
                        float lowFreq,  float lowGain,
                        float lmidFreq, float lmidGain,
                        float hmidFreq, float hmidGain,
                        float highFreq, float highGain);
        void setPhaser (bool on, float rate, float depth, float feedback, float mix);

    private:
        double sr       = 44100.0;
        int    maxBlock = 512;

        // ---- Drive ----
        bool  driveOn      = false;
        float driveAmount  = 1.0f;
        float driveTone    = 0.5f;
        float driveMix     = 0.0f;
        juce::dsp::IIR::Filter<float> toneL, toneR;
        std::vector<float> driveBufferScratch;

        // ---- Chorus ----
        bool  chorusOn     = false;
        float chorusRate   = 0.5f;
        float chorusDepth  = 0.25f;
        float chorusMix    = 0.5f;
        juce::dsp::Chorus<float> chorus;

        // ---- Delay (stereo con feedback manual) ----
        bool  delayOn      = false;
        float delayTimeSec = 0.3f;
        float delayFeedback= 0.4f;
        float delayMix     = 0.3f;
        juce::dsp::DelayLine<float> delayL { 192000 };
        juce::dsp::DelayLine<float> delayR { 192000 };

        // ---- Reverb ----
        bool  reverbOn     = false;
        float reverbMix    = 0.3f;
        juce::Reverb reverb;

        // ---- FASE 6.7: EQ 4 bandas (Low shelf, LMid peak, HMid peak, High shelf) ----
        using EQChain = juce::dsp::ProcessorChain<
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>>;

        bool  eqOn       = true;
        float eqLowFreq  = 100.0f,  eqLowGain  = 0.0f;
        float eqLmidFreq = 500.0f,  eqLmidGain = 0.0f;
        float eqHmidFreq = 2000.0f, eqHmidGain = 0.0f;
        float eqHighFreq = 8000.0f, eqHighGain = 0.0f;
        EQChain eqL, eqR;

        // ---- FASE 6.7: Phaser ----
        bool  phaserOn     = false;
        juce::dsp::Phaser<float> phaser;
    };
}
