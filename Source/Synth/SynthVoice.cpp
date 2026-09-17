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

        // --- Amp envelope ---
        adsrParams.attack  = getF (ParamIDs::ampAttack,  0.005f);
        adsrParams.decay   = getF (ParamIDs::ampDecay,   0.100f);
        adsrParams.sustain = getF (ParamIDs::ampSustain, 0.700f);
        adsrParams.release = getF (ParamIDs::ampRelease, 0.300f);
        adsr.setParameters (adsrParams);

        // --- Filter envelope ---
        filtAdsrParams.attack  = getF (ParamIDs::filtAttack,  0.005f);
        filtAdsrParams.decay   = getF (ParamIDs::filtDecay,   0.200f);
        filtAdsrParams.sustain = getF (ParamIDs::filtSustain, 0.500f);
        filtAdsrParams.release = getF (ParamIDs::filtRelease, 0.300f);
        filtAdsr.setParameters (filtAdsrParams);

        // --- Wavetables ---
        const int w1 = getI (ParamIDs::osc1Wave, 0);
        const int w2 = getI (ParamIDs::osc2Wave, 2);
        if (w1 != currentWave1Idx) { wave1 = dsp::wavetables::makeByIndex (w1); currentWave1Idx = w1; }
        if (w2 != currentWave2Idx) { wave2 = dsp::wavetables::makeByIndex (w2); currentWave2Idx = w2; }

        const float pos1   = getF (ParamIDs::osc1Pos,   0.0f);
        const float pos2   = getF (ParamIDs::osc2Pos,   0.0f);
        const float lvl1   = getF (ParamIDs::osc1Level, 0.8f);
        const float lvl2   = getF (ParamIDs::osc2Level, 0.0f);
        const float master = getF (ParamIDs::masterGain,0.7f);
        const int   oct1   = getI (ParamIDs::osc1Octave, 2) - 2;
        const int   oct2   = getI (ParamIDs::osc2Octave, 2) - 2;
        const float sem1   = getF (ParamIDs::osc1Semi, 0.0f);
        const float sem2   = getF (ParamIDs::osc2Semi, 0.0f);
        const float fin1   = getF (ParamIDs::osc1Fine, 0.0f);
        const float fin2   = getF (ParamIDs::osc2Fine, 0.0f);

        auto freqOf = [&] (int oct, float semis, float cents) noexcept
        {
            const float st = (float) oct * 12.0f + semis + cents / 100.0f;
            return currentFreq * std::pow (2.0f, st / 12.0f);
        };

        const float f1 = freqOf (oct1, sem1, fin1);
        const float f2 = freqOf (oct2, sem2, fin2);

        // --- Filter params ---
        const int   fType     = getI (ParamIDs::filterType,     0);
        const float fCutoff   = getF (ParamIDs::filterCutoff,   8000.0f);
        const float fReso     = getF (ParamIDs::filterReso,     0.0f);
        const float fEnvAmt   = getF (ParamIDs::filterEnvAmt,   0.0f);
        const float fKeyTrack = getF (ParamIDs::filterKeyTrack, 0.0f);

        filter.setType (fType);
        filter.setResonance (fReso);

        // Key tracking: desplaza el cutoff según la nota (0 = sin tracking, 1 = full)
        const float keyTrackedCutoff = fCutoff * std::pow (2.0f,
            fKeyTrack * (currentMidiNote - 60.0f) / 12.0f);

        auto* L = output.getWritePointer (0, startSample);
        auto* R = output.getNumChannels() > 1 ? output.getWritePointer (1, startSample) : nullptr;

        // Actualizamos el cutoff del filtro cada 32 muestras por eficiencia.
        constexpr int updateInterval = 32;

        for (int i = 0; i < numSamples; ++i)
        {
            const float env      = adsr.getNextSample();
            const float filtEnv  = filtAdsr.getNextSample();

            if (i % updateInterval == 0)
            {
                // envAmt en [-1,1] → modulación de hasta ±5 octavas
                const float modulated = keyTrackedCutoff * std::pow (2.0f, fEnvAmt * filtEnv * 5.0f);
                filter.setCutoff (modulated);
            }

            const float s1 = osc1.getNextSample (wave1, f1, pos1);
            const float s2 = osc2.getNextSample (wave2, f2, pos2);
            float mix = s1 * lvl1 + s2 * lvl2;

            mix = filter.processSample (mix);
            mix *= env * velocityGain * master;

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
