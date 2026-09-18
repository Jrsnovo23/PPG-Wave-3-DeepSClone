#include "SynthVoice.h"
#include "SynthSound.h"
#include "../ParameterIDs.h"
#include <cmath>

namespace synth
{
    static inline float noteToHz (int midiNote) noexcept
    {
        return 440.0f * std::pow (2.0f, (float) (midiNote - 69) / 12.0f);
    }

    SynthVoice::SynthVoice (juce::AudioProcessorValueTreeState& s) : apvts (s) {}

    bool SynthVoice::canPlaySound (juce::SynthesiserSound* s)
    {
        return dynamic_cast<SynthSound*> (s) != nullptr;
    }

    void SynthVoice::setCurrentPlaybackSampleRate (double newRate)
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
        if (newRate > 0.0)
        {
            osc1.prepare (newRate);
            osc2.prepare (newRate);
            filter.prepare (newRate);
            lfo1.prepare (newRate);
            lfo2.prepare (newRate);
            adsr.setSampleRate (newRate);
            adsr.setParameters (adsrParams);
            filtAdsr.setSampleRate (newRate);
            filtAdsr.setParameters (filtAdsrParams);
        }
    }

    void SynthVoice::startNote (int midiNote, float velocity,
                                juce::SynthesiserSound*, int)
    {
        currentFreq     = noteToHz (midiNote);
        currentMidiNote = (float) midiNote;
        velocityGain    = juce::jlimit (0.0f, 1.0f, velocity);

        osc1.reset (0.0f);
        osc2.reset (0.0f);
        filter.reset();
        lfo1.reset (0.0f);   // retrigger de fase en cada nota
        lfo2.reset (0.0f);
        adsr.noteOn();
        filtAdsr.noteOn();
        isActive = true;
    }

    void SynthVoice::stopNote (float, bool allowTailOff)
    {
        if (allowTailOff)
        {
            adsr.noteOff();
            filtAdsr.noteOff();
        }
        else
        {
            adsr.reset();
            filtAdsr.reset();
            clearCurrentNote();
            isActive = false;
        }
    }

    void SynthVoice::pitchWheelMoved (int newValue)
    {
        // 0..16383, centro en 8192 -> -1..1
        pitchBendValue = ((float) newValue - 8192.0f) / 8192.0f;
    }

    void SynthVoice::controllerMoved (int controllerNumber, int newValue)
    {
        const float v = (float) newValue / 127.0f;
        if (controllerNumber == 1)   modWheelValue = v;   // CC1
        if (controllerNumber == 128) aftertouchValue = v; // canal pressure (si el host lo envía)
    }

    void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& output,
                                      int startSample, int numSamples)
    {
        if (! isActive) return;

        auto getF = [&] (const char* id, float def) -> float
        {
            if (auto* p = apvts.getRawParameterValue (id)) return p->load();
            return def;
        };
        auto getI = [&] (const char* id, int def) -> int
        {
            if (auto* p = apvts.getRawParameterValue (id)) return (int) p->load();
            return def;
        };

        // ---------- Envolventes ----------
        adsrParams.attack  = getF (ParamIDs::ampAttack,  0.005f);
        adsrParams.decay   = getF (ParamIDs::ampDecay,   0.100f);
        adsrParams.sustain = getF (ParamIDs::ampSustain, 0.700f);
        adsrParams.release = getF (ParamIDs::ampRelease, 0.300f);
        adsr.setParameters (adsrParams);

        filtAdsrParams.attack  = getF (ParamIDs::filtAttack,  0.005f);
        filtAdsrParams.decay   = getF (ParamIDs::filtDecay,   0.200f);
        filtAdsrParams.sustain = getF (ParamIDs::filtSustain, 0.500f);
        filtAdsrParams.release = getF (ParamIDs::filtRelease, 0.300f);
        filtAdsr.setParameters (filtAdsrParams);

        // ---------- Wavetables ----------
        const int w1 = getI (ParamIDs::osc1Wave, 0);
        const int w2 = getI (ParamIDs::osc2Wave, 2);
        if (w1 != currentWave1Idx) { wave1 = dsp::wavetables::makeByIndex (w1); currentWave1Idx = w1; }
        if (w2 != currentWave2Idx) { wave2 = dsp::wavetables::makeByIndex (w2); currentWave2Idx = w2; }

        // ---------- Osciladores ----------
        const float basePos1 = getF (ParamIDs::osc1Pos,   0.0f);
        const float basePos2 = getF (ParamIDs::osc2Pos,   0.0f);
        const float lvl1     = getF (ParamIDs::osc1Level, 0.8f);
        const float lvl2     = getF (ParamIDs::osc2Level, 0.0f);
        const float master   = getF (ParamIDs::masterGain,0.7f);
        const int   oct1     = getI (ParamIDs::osc1Octave, 2) - 2;
        const int   oct2     = getI (ParamIDs::osc2Octave, 2) - 2;
        const float sem1     = getF (ParamIDs::osc1Semi, 0.0f);
        const float sem2     = getF (ParamIDs::osc2Semi, 0.0f);
        const float fin1     = getF (ParamIDs::osc1Fine, 0.0f);
        const float fin2     = getF (ParamIDs::osc2Fine, 0.0f);

        // Aplicamos pitch bend global (±2 semitonos)
        const float bendSemis = pitchBendValue * 2.0f;

        auto freqOf = [&] (int oct, float semis, float cents) noexcept
        {
            const float st = (float) oct * 12.0f + semis + cents / 100.0f + bendSemis;
            return currentFreq * std::pow (2.0f, st / 12.0f);
        };

        const float baseF1 = freqOf (oct1, sem1, fin1);
        const float baseF2 = freqOf (oct2, sem2, fin2);

        // ---------- Filtro ----------
        const int   fType      = getI (ParamIDs::filterType,     0);
        const float baseCutoff = getF (ParamIDs::filterCutoff,   8000.0f);
        const float baseReso   = getF (ParamIDs::filterReso,     0.0f);
        const float fEnvAmt    = getF (ParamIDs::filterEnvAmt,   0.0f);
        const float fKeyTrack  = getF (ParamIDs::filterKeyTrack, 0.0f);

        filter.setType (fType);

        const float keyTrackedCutoff = baseCutoff * std::pow (2.0f,
            fKeyTrack * (currentMidiNote - 60.0f) / 12.0f);

        // ---------- LFOs ----------
        const int   lfo1W = getI (ParamIDs::lfo1Wave,  0);
        const float lfo1R = getF (ParamIDs::lfo1Rate,  1.0f);
        const float lfo1D = getF (ParamIDs::lfo1Depth, 0.0f);
        const int   lfo2W = getI (ParamIDs::lfo2Wave,  0);
        const float lfo2R = getF (ParamIDs::lfo2Rate,  1.0f);
        const float lfo2D = getF (ParamIDs::lfo2Depth, 0.0f);

        const auto lfo1WaveEnum = (dsp::LFO::Wave) lfo1W;
        const auto lfo2WaveEnum = (dsp::LFO::Wave) lfo2W;

        // ---------- Matriz de modulación ----------
        ModSlot mods[4];
        mods[0] = { getI (ParamIDs::mod1Source, 0), getI (ParamIDs::mod1Dest, 0),
                    getF (ParamIDs::mod1Amount, 0.0f) };
        mods[1] = { getI (ParamIDs::mod2Source, 0), getI (ParamIDs::mod2Dest, 0),
                    getF (ParamIDs::mod2Amount, 0.0f) };
        mods[2] = { getI (ParamIDs::mod3Source, 0), getI (ParamIDs::mod3Dest, 0),
                    getF (ParamIDs::mod3Amount, 0.0f) };
        mods[3] = { getI (ParamIDs::mod4Source, 0), getI (ParamIDs::mod4Dest, 0),
                    getF (ParamIDs::mod4Amount, 0.0f) };

        auto* L = output.getWritePointer (0, startSample);
        auto* R = output.getNumChannels() > 1 ? output.getWritePointer (1, startSample) : nullptr;

        constexpr int updateInterval = 32;

        for (int i = 0; i < numSamples; ++i)
        {
            const float env     = adsr.getNextSample();
            const float filtEnv = filtAdsr.getNextSample();

            // LFOs: valor actual * depth
            const float lfo1Val = lfo1.getNextSample (lfo1WaveEnum, lfo1R, 0.0f) * lfo1D;
            const float lfo2Val = lfo2.getNextSample (lfo2WaveEnum, lfo2R, 0.0f) * lfo2D;

            // Key tracking normalizado (-1..1 aprox en 3 octavas)
            const float keyTrackVal = (currentMidiNote - 60.0f) / 36.0f;

            // Acumuladores por destino
            float modPitch1 = 0.0f;
            float modPitch2 = 0.0f;
            float modWT1    = 0.0f;
            float modWT2    = 0.0f;
            float modCutoff = 0.0f;
            float modReso   = 0.0f;
            float modAmp    = 1.0f;

            for (const auto& m : mods)
            {
                if (m.source == 0 || m.dest == 0 || m.amount == 0.0f) continue;

                float srcVal = 0.0f;
                switch (m.source)
                {
                    case 1: srcVal = lfo1Val;        break;
                    case 2: srcVal = lfo2Val;        break;
                    case 3: srcVal = env;            break;
                    case 4: srcVal = filtEnv;        break;
                    case 5: srcVal = velocityGain;   break;
                    case 6: srcVal = keyTrackVal;    break;
                    case 7: srcVal = modWheelValue;  break;
                    case 8: srcVal = aftertouchValue;break;
                    default: break;
                }

                const float v = srcVal * m.amount;

                switch (m.dest)
                {
                    case 1: modPitch1 += v; break;
                    case 2: modPitch2 += v; break;
                    case 3: modWT1    += v; break;
                    case 4: modWT2    += v; break;
                    case 5: modCutoff += v; break;
                    case 6: modReso   += v; break;
                    case 7: modAmp    += v; break;
                    default: break;
                }
            }

            // Aplicar modulaciones a los valores base
            const float f1   = baseF1 * std::pow (2.0f, modPitch1 * 2.0f);  // ±2 octavas por unidad
            const float f2   = baseF2 * std::pow (2.0f, modPitch2 * 2.0f);
            const float pos1 = juce::jlimit (0.0f, 1.0f, basePos1 + modWT1);
            const float pos2 = juce::jlimit (0.0f, 1.0f, basePos2 + modWT2);

            if (i % updateInterval == 0)
            {
                const float envMod   = std::pow (2.0f, fEnvAmt * filtEnv * 5.0f);
                const float modMod   = std::pow (2.0f, modCutoff * 5.0f);
                filter.setCutoff (keyTrackedCutoff * envMod * modMod);

                const float resoVal = juce::jlimit (0.0f, 1.0f, baseReso + modReso);
                filter.setResonance (resoVal);
            }

            // Generación de la muestra
            const float s1 = osc1.getNextSample (wave1, f1, pos1);
            const float s2 = osc2.getNextSample (wave2, f2, pos2);
            float mix = s1 * lvl1 + s2 * lvl2;

            mix = filter.processSample (mix);
            mix *= env * velocityGain * master * juce::jlimit (0.0f, 2.0f, modAmp);

            L[i] += mix;
            if (R) R[i] += mix;
        }

        if (! adsr.isActive())
        {
            clearCurrentNote();
            isActive = false;
        }
    }
}
