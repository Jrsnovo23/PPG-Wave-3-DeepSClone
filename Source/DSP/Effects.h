#pragma once
#include <juce_dsp/juce_dsp.h>

namespace dsp
{
    // Cadena de efectos globales aplicada post-mix.
    // Orden: Drive -> Chorus -> Phaser -> Delay -> Reverb -> EQ -> Vintage (DAC).
    class Effects
    {
    public:
        void prepare (double sampleRate, int maxBlockSize, int numChannels);
        void reset();
        void process (juce::AudioBuffer<float>& buffer);

        void setChorus (bool on, float rate, float depth, float mix);
        void setDelay  (bool on, float timeSec, float feedback, float mix);
        void setReverb (bool on, float roomSize, float damping, float mix);
        void setDrive  (bool on, float drive, float tone, float mix);

        void setEQ     (bool on,
                        bool hpOn, bool lpOn,
                        float lowFreq,  float lowQ,  float lowGain,
                        float lmidFreq, float lmidQ, float lmidGain,
                        float hmidFreq, float hmidQ, float hmidGain,
                        float highFreq, float highQ, float highGain);
        void setPhaser (bool on, float rate, float depth, float feedback, float mix);

        // ===== FASE 8 =====
        void setVintage (bool on, float amount,
                         float bits, float srFactor,
                         float noise);

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

        // ---- Delay ----
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

        // ---- EQ ----
        using EQChain = juce::dsp::ProcessorChain<
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>,
            juce::dsp::IIR::Filter<float>>;

        bool  eqOn   = true;
        bool  eqHpOn = false;
        bool  eqLpOn = false;

        float eqLowFreq  = 100.0f,  eqLowQ  = 0.707f, eqLowGain  = 0.0f;
        float eqLmidFreq = 500.0f,  eqLmidQ = 0.707f, eqLmidGain = 0.0f;
        float eqHmidFreq = 2000.0f, eqHmidQ = 0.707f, eqHmidGain = 0.0f;
        float eqHighFreq = 8000.0f, eqHighQ = 0.707f, eqHighGain = 0.0f;

        EQChain eqL, eqR;

        // ---- Phaser ----
        bool  phaserOn     = false;
        juce::dsp::Phaser<float> phaser;

        // ---- FASE 8: Vintage Character ----
        bool  vintageOn     = false;
        float vintageAmount = 0.5f;
        float vintageBits   = 12.0f;
        float vintageSr     = 1.0f;
        float vintageNoise  = 0.15f;

        // Estado del sample & hold
        float heldL    = 0.0f;
        float heldR    = 0.0f;
        float srCounter = 1.0f;

        juce::Random vintageRng;
    };
}
