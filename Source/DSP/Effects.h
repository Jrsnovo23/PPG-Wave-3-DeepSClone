#pragma once
#include <juce_dsp/juce_dsp.h>

namespace dsp
{
    // Cadena de efectos globales aplicada post-mix.
    // Orden: EQ (HP -> LowShelf -> LMid -> HMid -> HighShelf -> LP)
    //        -> Drive -> Chorus -> Phaser -> Delay -> Reverb.
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

        // ===== FASE 6.7 =====
        void setEQ     (bool on,
                        bool hpOn, bool lpOn,
                        float lowFreq,  float lowQ,  float lowGain,
                        float lmidFreq, float lmidQ, float lmidGain,
                        float hmidFreq, float hmidQ, float hmidGain,
                        float highFreq, float highQ, float highGain);
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

        // ---- FASE 6.7: EQ 4 bandas + HP/LP ----
        // Chain: [0]=HP, [1]=LowShelf, [2]=LmidPeak, [3]=HmidPeak, [4]=HighShelf, [5]=LP
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

        // ---- FASE 6.7: Phaser ----
        bool  phaserOn     = false;
        juce::dsp::Phaser<float> phaser;
    };
}
