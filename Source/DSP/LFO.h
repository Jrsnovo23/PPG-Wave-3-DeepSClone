#pragma once
#include <juce_core/juce_core.h>
#include <cmath>

namespace dsp
{
    // LFO con 6 formas: Sine, Triangle, Saw, Square, Random, Sample & Hold.
    // Sync: cuando sync=true, la frecuencia se calcula a partir del BPM del host.
    class LFO
    {
    public:
        enum class Wave { Sine = 0, Triangle, Saw, Square, Random, SampleHold };

        void prepare (double sampleRate) noexcept { sr = sampleRate; }
        void reset   (float initialPhase = 0.0f) noexcept
        {
            phase = initialPhase;
            rngState = 0x12345678u;
            lastRandom = 0.0f;
            lastSH = 0.0f;
        }

        // rateHz: frecuencia libre (si sync=false) o múltiplo del BPM (si sync=true).
        // phaseOffset en [0,1) para permitir desfase entre LFOs.
        float getNextSample (Wave wave, float rateHz, float phaseOffset = 0.0f) noexcept;

    private:
        double sr = 44100.0;
        float  phase = 0.0f;
        juce::uint32 rngState = 0x12345678u;
        float  lastRandom = 0.0f;
        float  lastSH = 0.0f;

        float nextRandom() noexcept;
    };
}
