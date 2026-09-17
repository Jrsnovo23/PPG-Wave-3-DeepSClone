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
            adsr.setSampleRate (newRate);
            adsr.setParameters (adsrParams);
        }
    }

    void SynthVoice::startNote (int midiNote, float velocity,
                                juce::SynthesiserSound*, int)
    {
        currentFreq  = noteToHz (midiNote);
        velocityGain = juce::jlimit (0.0f, 1.0f, velocity);
        osc1.reset (0.0f);
        osc2.reset (0.0f);
        adsr.noteOn();
        isActive = true;
    }

    void SynthVoice::stopNote (float, bool allowTailOff)
    {
        if (allowTailOff) { adsr.noteOff(); }
        else              { adsr.reset(); clearCurrentNote(); isActive = false; }
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

        // Actualizar ADSR desde parámetros (barato: se hace una vez por bloque)
        adsrParams.attack  = getF (ParamIDs::ampAttack,  0.005f);
        adsrParams.decay   = getF (ParamIDs::ampDecay,   0.100f);
        adsrParams.sustain = getF (ParamIDs::ampSustain, 0.700f);
        adsrParams.release = getF (ParamIDs::ampRelease, 0.300f);
        adsr.setParameters (adsrParams);

        const int w1 = getI (ParamIDs::osc1Wave, 0);
        const int w2 = getI (ParamIDs::osc2Wave, 2);
        if (w1 != currentWave1Idx) { wave1 = dsp::wavetables::makeByIndex (w1); currentWave1Idx = w1; }
        if (w2 != currentWave2Idx) { wave2 = dsp::wavetables::makeByIndex (w2); currentWave2Idx = w2; }

        const float pos1  = getF (ParamIDs::osc1Pos,   0.0f);
        const float pos2  = getF (ParamIDs::osc2Pos,   0.0f);
        const float lvl1  = getF (ParamIDs::osc1Level, 0.8f);
        const float lvl2  = getF (ParamIDs::osc2Level, 0.0f);
        const float master= getF (ParamIDs::masterGain,0.7f);
        const int   oct1  = getI (ParamIDs::osc1Octave, 2) - 2;
        const int   oct2  = getI (ParamIDs::osc2Octave, 2) - 2;
        const float sem1  = getF (ParamIDs::osc1Semi, 0.0f);
        const float sem2  = getF (ParamIDs::osc2Semi, 0.0f);
        const float fin1  = getF (ParamIDs::osc1Fine, 0.0f);
        const float fin2  = getF (ParamIDs::osc2Fine, 0.0f);

        auto freqOf = [&] (int oct, float semis, float cents) noexcept
        {
            const float st = (float) oct * 12.0f + semis + cents / 100.0f;
            return currentFreq * std::pow (2.0f, st / 12.0f);
        };

        const float f1 = freqOf (oct1, sem1, fin1);
        const float f2 = freqOf (oct2, sem2, fin2);

        auto* L = output.getWritePointer (0, startSample);
        auto* R = output.getNumChannels() > 1 ? output.getWritePointer (1, startSample) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            const float env = adsr.getNextSample();
            const float s1  = osc1.getNextSample (wave1, f1, pos1);
            const float s2  = osc2.getNextSample (wave2, f2, pos2);
            const float mix = (s1 * lvl1 + s2 * lvl2) * env * velocityGain * master;
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
