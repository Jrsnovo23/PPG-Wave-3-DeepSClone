#include "Effects.h"

namespace dsp
{
    void Effects::prepare (double sampleRate, int maxBlockSize, int /*numChannels*/)
    {
        sr       = sampleRate;
        maxBlock = maxBlockSize;

        toneL.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
        toneR.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 1 });
        toneL.reset();
        toneR.reset();

        chorus.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 2 });
        chorus.reset();

        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 1 };
        delayL.prepare (spec);
        delayR.prepare (spec);
        delayL.setMaximumDelayInSamples ((int) (sampleRate * 2.0));
        delayR.setMaximumDelayInSamples ((int) (sampleRate * 2.0));
        delayL.reset();
        delayR.reset();

        reverb.setSampleRate (sampleRate);

        {
            juce::dsp::ProcessSpec monoSpec { sampleRate, (juce::uint32) maxBlockSize, 1 };
            eqL.prepare (monoSpec);
            eqR.prepare (monoSpec);
            eqL.reset();
            eqR.reset();

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
            auto setFlatPeak = [] (juce::dsp::IIR::Filter<float>& f, double sr_, float freq, float q)
            {
                *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                    sr_, freq, q, 1.0f);
            };
            auto setFlatHP = [] (juce::dsp::IIR::Filter<float>& f, double sr_, float freq)
            {
                *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (
                    sr_, freq, 0.707f);
            };
            auto setFlatLP = [] (juce::dsp::IIR::Filter<float>& f, double sr_, float freq)
            {
                *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
                    sr_, freq, 0.707f);
            };

            for (auto* chain : { &eqL, &eqR })
            {
                setFlatHP       (chain->get<0>(), sampleRate, eqLowFreq);
                setFlatShelf    (chain->get<1>(), sampleRate, eqLowFreq);
                setFlatPeak     (chain->get<2>(), sampleRate, eqLmidFreq, eqLmidQ);
                setFlatPeak     (chain->get<3>(), sampleRate, eqHmidFreq, eqHmidQ);
                setFlatHighShelf(chain->get<4>(), sampleRate, eqHighFreq);
                setFlatLP       (chain->get<5>(), sampleRate, eqHighFreq);
            }
        }

        phaser.prepare ({ sampleRate, (juce::uint32) maxBlockSize, 2 });
        phaser.reset();
        phaser.setCentreFrequency (1000.0f);

        // FASE 8
        heldL = 0.0f;
        heldR = 0.0f;
        srCounter = 1.0f;
        vintageRng.setSeed (0x5A17F00Du);

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

        // FASE 8
        heldL = 0.0f;
        heldR = 0.0f;
        srCounter = 1.0f;
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

    void Effects::setEQ (bool on,
                         bool hpOn, bool lpOn,
                         float lowFreq,  float lowQ,  float lowGain,
                         float lmidFreq, float lmidQ, float lmidGain,
                         float hmidFreq, float hmidQ, float hmidGain,
                         float highFreq, float highQ, float highGain)
    {
        eqOn   = on;
        eqHpOn = hpOn;
        eqLpOn = lpOn;

        const bool changed =
            (lowFreq  != eqLowFreq)  || (lowQ  != eqLowQ)  || (lowGain  != eqLowGain)  ||
            (lmidFreq != eqLmidFreq) || (lmidQ != eqLmidQ) || (lmidGain != eqLmidGain) ||
            (hmidFreq != eqHmidFreq) || (hmidQ != eqHmidQ) || (hmidGain != eqHmidGain) ||
            (highFreq != eqHighFreq) || (highQ != eqHighQ) || (highGain != eqHighGain);

        eqLowFreq  = lowFreq;  eqLowQ  = lowQ;  eqLowGain  = lowGain;
        eqLmidFreq = lmidFreq; eqLmidQ = lmidQ; eqLmidGain = lmidGain;
        eqHmidFreq = hmidFreq; eqHmidQ = hmidQ; eqHmidGain = hmidGain;
        eqHighFreq = highFreq; eqHighQ = highQ; eqHighGain = highGain;

        if (! changed || sr <= 0.0) return;

        const float lowGainLin  = juce::Decibels::decibelsToGain (eqLowGain);
        const float lmidGainLin = juce::Decibels::decibelsToGain (eqLmidGain);
        const float hmidGainLin = juce::Decibels::decibelsToGain (eqHmidGain);
        const float highGainLin = juce::Decibels::decibelsToGain (eqHighGain);

        const float clampedLmidQ = juce::jlimit (0.1f, 10.0f, eqLmidQ);
        const float clampedHmidQ = juce::jlimit (0.1f, 10.0f, eqHmidQ);

        for (auto* chain : { &eqL, &eqR })
        {
            *chain->get<0>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (
                sr, eqLowFreq, 0.707f);
            *chain->get<1>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
                sr, eqLowFreq, 0.707f, lowGainLin);
            *chain->get<2>().coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                sr, eqLmidFreq, clampedLmidQ, lmidGainLin);
            *chain->get<3>().coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                sr, eqHmidFreq, clampedHmidQ, hmidGainLin);
            *chain->get<4>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
                sr, eqHighFreq, 0.707f, highGainLin);
            *chain->get<5>().coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass (
                sr, eqHighFreq, 0.707f);
        }
    }

    void Effects::setPhaser (bool on, float rate, float depth, float feedback, float mix)
    {
        phaserOn = on;
        phaser.setRate     (juce::jlimit (0.01f, 10.0f, rate));
        phaser.setDepth    (juce::jlimit (0.0f, 1.0f, depth));
        phaser.setFeedback (juce::jlimit (0.0f, 0.95f, feedback));
        phaser.setMix      (juce::jlimit (0.0f, 1.0f, mix));
    }

    // ==================== FASE 8: Vintage ====================
    void Effects::setVintage (bool on, float amount,
                              float bits, float srFactor,
                              float noise)
    {
        vintageOn     = on;
        vintageAmount = juce::jlimit (0.0f, 1.0f, amount);
        vintageBits   = juce::jlimit (4.0f, 16.0f, bits);
        vintageSr     = juce::jlimit (1.0f, 32.0f, srFactor);
        vintageNoise  = juce::jlimit (0.0f, 1.0f, noise);
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

        // ---------- 3. PHASER ----------
        if (phaserOn && R)
        {
            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> ctx (block);
            phaser.process (ctx);
        }

        // ---------- 4. DELAY ----------
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

        // ---------- 5. REVERB ----------
        if (reverbOn && reverbMix > 0.0f)
        {
            if (R)
                reverb.processStereo (L, R, numSamples);
            else
                reverb.processMono (L, numSamples);
        }

        // ---------- 6. EQ ----------
        if (eqOn)
        {
            {
                float* chansL[] = { L };
                juce::dsp::AudioBlock<float> bL (chansL, 1, 0, (size_t) numSamples);
                juce::dsp::ProcessContextReplacing<float> ctx (bL);
                if (eqHpOn) eqL.get<0>().process (ctx);
                eqL.get<1>().process (ctx);
                eqL.get<2>().process (ctx);
                eqL.get<3>().process (ctx);
                eqL.get<4>().process (ctx);
                if (eqLpOn) eqL.get<5>().process (ctx);
            }
            if (R)
            {
                float* chansR[] = { R };
                juce::dsp::AudioBlock<float> bR (chansR, 1, 0, (size_t) numSamples);
                juce::dsp::ProcessContextReplacing<float> ctx (bR);
                if (eqHpOn) eqR.get<0>().process (ctx);
                eqR.get<1>().process (ctx);
                eqR.get<2>().process (ctx);
                eqR.get<3>().process (ctx);
                eqR.get<4>().process (ctx);
                if (eqLpOn) eqR.get<5>().process (ctx);
            }
        }

        // ---------- 7. FASE 8: VINTAGE (DAC emulation, al final) ----------
        if (vintageOn && vintageAmount > 0.0f)
        {
            const float amt = vintageAmount;

            // Interpolación: 16 bits en amt=0, vintageBits en amt=1
            const float effBits = 16.0f - amt * (16.0f - vintageBits);
            const float effSr   = 1.0f  + amt * (vintageSr - 1.0f);
            const float effNoise = amt * vintageNoise;

            const float levels    = std::pow (2.0f, effBits);
            const float invLevels = 1.0f / levels;
            const float srStep    = 1.0f / juce::jmax (1.0f, effSr);
            const float noiseAmt  = effNoise * 0.02f;

            for (int i = 0; i < numSamples; ++i)
            {
                // Sample & hold
                srCounter += srStep;
                if (srCounter >= 1.0f)
                {
                    heldL = L[i];
                    if (R) heldR = R[i];
                    srCounter -= 1.0f;
                }

                float sL = heldL;
                float sR = R ? heldR : 0.0f;

                // Bit reduction (DAC emulation)
                sL = std::round (sL * levels) * invLevels;
                if (R) sR = std::round (sR * levels) * invLevels;

                // Digital noise
                if (noiseAmt > 0.0f)
                {
                    sL += (vintageRng.nextFloat() * 2.0f - 1.0f) * noiseAmt;
                    if (R) sR += (vintageRng.nextFloat() * 2.0f - 1.0f) * noiseAmt;
                }

                L[i] = sL;
                if (R) R[i] = sR;
            }
        }
    }
}
