#include "LFO.h"

namespace dsp
{
    float LFO::nextRandom() noexcept
    {
        // xorshift32 rápido, sin allocaciones.
        rngState ^= rngState << 13;
        rngState ^= rngState >> 17;
        rngState ^= rngState << 5;
        return ((float) (rngState & 0xFFFFu) / 32768.0f) - 1.0f; // [-1, 1]
    }

    float LFO::getNextSample (Wave wave, float rateHz, float phaseOffset) noexcept
    {
        const float p = std::fmod (phase + phaseOffset, 1.0f);
        float out = 0.0f;

        switch (wave)
        {
            case Wave::Sine:
                out = std::sin (p * juce::MathConstants<float>::twoPi);
                break;

            case Wave::Triangle:
                out = 4.0f * std::abs (p - 0.5f) - 1.0f;
                break;

            case Wave::Saw:
                out = 2.0f * p - 1.0f;
                break;

            case Wave::Square:
                out = (p < 0.5f) ? 1.0f : -1.0f;
                break;

            case Wave::Random:
                if (phase > p)             // wrap detectado: nueva muestra aleatoria
                    lastRandom = nextRandom();
                out = lastRandom;
                break;

            case Wave::SampleHold:
                if (phase > p)
                    lastSH = nextRandom();
                out = lastSH;
                break;
        }

        // Avance de fase
        phase += (float) ((double) rateHz / sr);
        phase -= std::floor (phase);
        return out;
    }
}
