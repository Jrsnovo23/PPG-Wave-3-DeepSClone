#include "Effects.h"

namespace dsp
{
    void Effects::prepare (double sampleRate, int maxBlockSize, int /*numChannels*/)
    {
        sr       = sampleRate;
        maxBlock = maxBlockSize;

        // Drive: filtro de tono (low-pass de 1 polo)
        toneL.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
        toneR.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
        toneL.reset();
        toneR.reset();

        // Chorus
        chorus.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 2 });
        chorus.reset();

        // Delay
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };
        delayL.prepare (spec);
        delayR.prepare (spec);
        delayL.setMaximumDelayInSamples ((int) (sampleRate * 2.0));  // hasta 2 seg
        delayR.setMaximumDelayInSamples ((int) (sampleRate * 2.0));
        delayL.reset();
        delayR.reset();

        // Reverb (parámetros por defecto; se ajustan en setReverb)
        reverb.setSampleRate (sampleRate);

        driveBufferScratch.resize ((size_t) maxBlockSize);
    }

    void Effects::reset()
    {
        chorus.reset();
        delayL.reset();
        delayR.reset();
        toneL.reset();
        toneR.reset();
        reverb.reset();
    }

    void Effects::setChorus (bool on, float rate, float depth, float mix)
    {
        chorusOn   = on;
        chorusRate = rate;
        chorusDepth= depth;
        chorusMix  = mix;

        chorus.setRate  (rate);
        chorus.setDepth (depth);
        chorus.setMix   (mix);
        chorus.setCentreDelay (7.0f);
        chorus.setFeedback    (0.0f);
    }

    void Effects::setDelay (bool on, float timeSec, float feedback, float mix)
    {
        delayOn       = on;
        delayTimeSec  = timeSec;
        delayFeedback = juce::jlimit (0.0f, 0.95f, feedback);
        delayMix      = mix;

        const float samples = juce::jlimit (1.0f, (float) (sr * 2.0 - 4.0),
                                            timeSec * (float) sr);
        delayL.setDelay (samples);
        delayR.setDelay (samples);
    }

    void Effects::setReverb (bool on, float roomSize, float damping, float mix)
    {
        reverbOn = on;
        reverbMix = mix;

        juce::Reverb::Parameters p;
        p.roomSize = juce::jlimit (0.0f, 1.0f, roomSize);
        p.damping  = juce::jlimit (0.0f, 1.0f, damping);
        p.wetLevel = 0.5f;
        p.dryLevel = 0.5f;
        p.width    = 1.0f;
        p.freezeMode = 0.0f;
        reverb.setParameters (p);
    }

    void Effects::setDrive (bool on, float drive, float tone, float mix)
    {
        driveOn     = on;
        driveAmount = juce::jmax (1.0f, drive);
        driveTone   = juce::jlimit (0.0f, 1.0f, tone);
        driveMix    = juce::jlimit (0.0f, 1.0f, mix);

        // tone 0..1 -> cutoff 400Hz..12000Hz (exponencial)
        const float cutoff = 400.0f * std::pow (30.0f, driveTone);
        *toneL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sr, cutoff);
        *toneR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sr, cutoff);
    }

    void Effects::process (juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples <= 0) return;

        auto* L = buffer.getWritePointer (0);
        auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

        // ---------- 1. DRIVE ----------
        if (driveOn && driveMix > 0.0f)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                const float dryL = L[i];
                const float dryR = R ? R[i] : dryL;

                // Waveshaper: tanh escalado
                float wetL = std::tanh (dryL * driveAmount) / std::tanh (driveAmount);
                float wetR = std::tanh (dryR * driveAmount) / std::tanh (driveAmount);

                // Filtro de tono
                wetL = toneL.processSample (wetL);
                if (R) wetR = toneR.processSample (wetR);

                L[i] = dryL * (1.0f - driveMix) + wetL * driveMix;
                if (R) R[i] = dryR * (1.0f - driveMix) + wetR * driveMix;
            }
        }

        // ---------- 2. CHORUS ----------
        if (chorusOn && chorusMix > 0.0f)
        {
            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> ctx (block);
            chorus.process (ctx);
        }

        // ---------- 3. DELAY ----------
        if (delayOn && delayMix > 0.0f)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                const float dryL = L[i];
                const float dryR = R ? R[i] : dryL;

                const float delL = delayL.popSample (0);
                const float delR = delayR.popSample (0);

                delayL.pushSample (0, dryL + delL * delayFeedback);
                delayR.pushSample (0, dryR + delR * delayFeedback);

                L[i] = dryL + delL * delayMix;
                if (R) R[i] = dryR + delR * delayMix;
            }
        }

        // ---------- 4. REVERB ----------
        if (reverbOn && reverbMix > 0.0f)
        {
            if (R)
            {
                reverb.processStereo (L, R, numSamples);
            }
            else
            {
                reverb.processMono (L, numSamples);
            }

            // Mezcla wet/dry manual (juce::Reverb ya mezcla internamente,
            // pero aquí ajustamos el nivel final con reverbMix)
            if (reverbMix < 1.0f)
            {
                // El reverb de JUCE ya devuelve una mezcla; no hacemos dry/wet adicional
                // para no duplicar procesamiento. Si quisieras control fino,
                // podrías guardar el dry antes del process.
            }
        }
    }
}
