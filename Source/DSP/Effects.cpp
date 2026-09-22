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
        delayL.setMaximumDelayInSamples ((int) (sampleRate * 2.0));
        delayR.setMaximumDelayInSamples ((int) (sampleRate * 2.0));
        delayL.reset();
        delayR.reset();

        // Reverb
        reverb.setSampleRate (sampleRate);

        // FASE 6.7: EQ (uno por canal, mono specs)
        {
            juce::dsp::ProcessSpec monoSpec { sampleRate, (juce::uint32) maxBlockSize, 1 };
            eqL.prepare (monoSpec);
            eqR.prepare (monoSpec);
            eqL.reset();
            eqR.reset();

            // Coeficientes iniciales planos (0 dB)
            auto setFlatShelf = [] (juce::dsp::IIR::Filter<float>& f, double sr_, float freq)
            {
                *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
                    sr_, freq, 0.707f, 1.0f);
            };
            auto setFlatHighShelf = [] (juce::dsp::IIR::Filter<float>& f, double sr_, float freq)
            {
                *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
                    sr_, freq, 0.707f, 1.0f);
            };
            auto setFlatPeak = [] (juce::dsp::IIR::Filter<float>& f, double sr_, float freq)
            {
                *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                    sr_, freq, 0.707f, 1.0f);
            };
            setFlatShelf    (eqL.get<0>(), sampleRate, eqLowFreq);
            setFlatPeak     (eqL.get<1>(), sampleRate, eqLmidFreq);
            setFlatPeak     (eqL.get<2>(), sampleRate, eqHmidFreq);
            setFlatHighShelf(eqL.get<3>(), sampleRate, eqHighFreq);
            setFlatShelf    (eqR.get<0>(), sampleRate, eqLowFreq);
            setFlatPeak     (eqR.get<1>(), sampleRate, eqLmidFreq);
            setFlatPeak     (eqR.get<2>(), sampleRate, eqHmidFreq);
            setFlatHighShelf(eqR.get<3>(), sampleRate, eqHighFreq);
        }

        // FASE 6.7: Phaser
        phaser.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 2 });
        phaser.reset();
        phaser.setCentreFrequency (1000.0f);

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
        eqL.reset();
        eqR.reset();
        phaser.reset();
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

        const float cutoff = 400.0f * std::pow (30.0f, driveTone);
        *toneL.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sr, cutoff);
        *toneR.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sr, cutoff);
    }

    // ==================== FASE 6.7: EQ ====================
    void Effects::setEQ (bool on,
                         float lowFreq,  float lowGain,
                         float lmidFreq, float lmidGain,
                         float hmidFreq, float hmidGain,
                         float highFreq, float highGain)
    {
        eqOn = on;

        // Solo regeneramos coeficientes si algo ha cambiado.
        const bool changed =
            (lowFreq  != eqLowFreq)  || (lowGain  != eqLowGain)  ||
            (lmidFreq != eqLmidFreq) || (lmidGain != eqLmidGain) ||
            (hmidFreq != eqHmidFreq) || (hmidGain != eqHmidGain) ||
            (highFreq != eqHighFreq) || (highGain != eqHighGain);

        eqLowFreq  = lowFreq;  eqLowGain  = lowGain;
        eqLmidFreq = lmidFreq; eqLmidGain = lmidGain;
        eqHmidFreq = hmidFreq; eqHmidGain = hmidGain;
        eqHighFreq = highFreq; eqHighGain = highGain;

        if (! changed || sr <= 0.0) return;

        const float lowGainLin  = juce::Decibels::decibelsToGain (eqLowGain);
        const float lmidGainLin = juce::Decibels::decibelsToGain (eqLmidGain);
        const float hmidGainLin = juce::Decibels::decibelsToGain (eqHmidGain);
        const float highGainLin = juce::Decibels::decibelsToGain (eqHighGain);

        auto setLowShelf = [&] (juce::dsp::IIR::Filter<float>& f)
        {
            *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
                sr, eqLowFreq, 0.707f, lowGainLin);
        };
        auto setLmidPeak = [&] (juce::dsp::IIR::Filter<float>& f)
        {
            *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                sr, eqLmidFreq, 0.707f, lmidGainLin);
        };
        auto setHmidPeak = [&] (juce::dsp::IIR::Filter<float>& f)
        {
            *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                sr, eqHmidFreq, 0.707f, hmidGainLin);
        };
        auto setHighShelf = [&] (juce::dsp::IIR::Filter<float>& f)
        {
            *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
                sr, eqHighFreq, 0.707f, highGainLin);
        };

        setLowShelf  (eqL.get<0>());
        setLmidPeak  (eqL.get<1>());
        setHmidPeak  (eqL.get<2>());
        setHighShelf (eqL.get<3>());

        setLowShelf  (eqR.get<0>());
        setLmidPeak  (eqR.get<1>());
        setHmidPeak  (eqR.get<2>());
        setHighShelf (eqR.get<3>());
    }

    // ==================== FASE 6.7: Phaser ====================
    void Effects::setPhaser (bool on, float rate, float depth, float feedback, float mix)
    {
        phaserOn = on;

        phaser.setRate     (juce::jlimit (0.01f, 10.0f, rate));
        phaser.setDepth    (juce::jlimit (0.0f, 1.0f, depth));
        phaser.setFeedback (juce::jlimit (0.0f, 0.95f, feedback));
        phaser.setMix      (juce::jlimit (0.0f, 1.0f, mix));
    }

    void Effects::process (juce::AudioBuffer<float>& buffer)
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples <= 0) return;

        auto* L = buffer.getWritePointer (0);
        auto* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

        // ---------- 0. EQ (FASE 6.7) ----------
        if (eqOn)
        {
            {
                float* chansL[] = { L };
                juce::dsp::AudioBlock<float> blockL (chansL, 1, 0, (size_t) numSamples);
                juce::dsp::ProcessContextReplacing<float> ctxL (blockL);
                eqL.process (ctxL);
            }
            if (R)
            {
                float* chansR[] = { R };
                juce::dsp::AudioBlock<float> blockR (chansR, 1, 0, (size_t) numSamples);
                juce::dsp::ProcessContextReplacing<float> ctxR (blockR);
                eqR.process (ctxR);
            }
        }

        // ---------- 1. DRIVE ----------
        if (driveOn && driveMix > 0.0f)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                const float dryL = L[i];
                const float dryR = R ? R[i] : dryL;

                float wetL = std::tanh (dryL * driveAmount) / std::tanh (driveAmount);
                float wetR = std::tanh (dryR * driveAmount) / std::tanh (driveAmount);

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

        // ---------- 2b. PHASER (FASE 6.7) ----------
        if (phaserOn && R)
        {
            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> ctx (block);
            phaser.process (ctx);
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
                reverb.processStereo (L, R, numSamples);
            else
                reverb.processMono (L, numSamples);
        }
    }
}
