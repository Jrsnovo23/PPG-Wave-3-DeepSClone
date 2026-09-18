#include "ParameterLayout.h"
#include "../ParameterIDs.h"

namespace Params
{
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        auto makeChoice = [&] (const juce::String& id, const juce::String& name,
                               const juce::StringArray& items, int def)
        {
            params.push_back (std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { id, 1 }, name, items, def));
        };

        auto makeFloat = [&] (const juce::String& id, const juce::String& name,
                              float lo, float hi, float def,
                              const juce::String& suffix = {})
        {
            params.push_back (std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { id, 1 }, name,
                juce::NormalisableRange<float> { lo, hi }, def,
                juce::AudioParameterFloatAttributes().withLabel (suffix)));
        };

        auto makeFloatLog = [&] (const juce::String& id, const juce::String& name,
                                 float lo, float hi, float def,
                                 const juce::String& suffix = {})
        {
            juce::NormalisableRange<float> range (lo, hi);
            range.setSkewForCentre (std::sqrt (lo * hi));
            params.push_back (std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { id, 1 }, name, range, def,
                juce::AudioParameterFloatAttributes().withLabel (suffix)));
        };

        // ============ Listas de opciones ============
        const juce::StringArray waveChoices   { "Sine", "Triangle", "Saw", "Square" };
        const juce::StringArray octaveChoices { "-2", "-1", "0", "+1", "+2" };
        const juce::StringArray filterTypes   { "Low Pass", "High Pass", "Band Pass" };

        const juce::StringArray lfoWaves {
            "Sine", "Triangle", "Square", "Saw Up", "Saw Down", "Random", "Sample & Hold"
        };

        const juce::StringArray modSources {
            "None", "LFO 1", "LFO 2", "Envelope 1", "Envelope 2",
            "Velocity", "Mod Wheel", "Aftertouch", "Note Number", "Random"
        };

        const juce::StringArray modDests {
            "None", "OSC1 Pitch", "OSC2 Pitch", "OSC1 Wave Pos", "OSC2 Wave Pos",
            "Filter Cutoff", "Amplifier"
        };

        // ============ Oscilador 1 ============
        makeChoice (ParamIDs::osc1Wave,   "Osc1 Wave",   waveChoices,   0);
        makeFloat  (ParamIDs::osc1Pos,    "Osc1 Pos",    0.0f, 1.0f, 0.0f);
        makeChoice (ParamIDs::osc1Octave, "Osc1 Octave", octaveChoices, 2);
        makeFloat  (ParamIDs::osc1Semi,   "Osc1 Semi",  -12.0f, 12.0f, 0.0f, "st");
        makeFloat  (ParamIDs::osc1Fine,   "Osc1 Fine", -100.0f, 100.0f, 0.0f, "ct");
        makeFloat  (ParamIDs::osc1Level,  "Osc1 Level",  0.0f, 1.0f, 0.8f);

        // ============ Oscilador 2 ============
        makeChoice (ParamIDs::osc2Wave,   "Osc2 Wave",   waveChoices,   2);
        makeFloat  (ParamIDs::osc2Pos,    "Osc2 Pos",    0.0f, 1.0f, 0.0f);
        makeChoice (ParamIDs::osc2Octave, "Osc2 Octave", octaveChoices, 2);
        makeFloat  (ParamIDs::osc2Semi,   "Osc2 Semi",  -12.0f, 12.0f, 0.0f, "st");
        makeFloat  (ParamIDs::osc2Fine,   "Osc2 Fine", -100.0f, 100.0f, 0.0f, "ct");
        makeFloat  (ParamIDs::osc2Level,  "Osc2 Level",  0.0f, 1.0f, 0.0f);

        // ============ Amp Envelope ============
        makeFloat (ParamIDs::ampAttack,  "Amp Attack",  0.001f, 5.0f, 0.005f, "s");
        makeFloat (ParamIDs::ampDecay,   "Amp Decay",   0.001f, 5.0f, 0.100f, "s");
        makeFloat (ParamIDs::ampSustain, "Amp Sustain", 0.0f,   1.0f, 0.700f);
        makeFloat (ParamIDs::ampRelease, "Amp Release", 0.001f, 10.0f, 0.300f, "s");

        // ============ Filtro ============
        makeChoice   (ParamIDs::filterType,     "Filter Type",     filterTypes, 0);
        makeFloatLog (ParamIDs::filterCutoff,   "Filter Cutoff",   20.0f, 20000.0f, 8000.0f, "Hz");
        makeFloat    (ParamIDs::filterReso,     "Filter Reso",     0.0f, 1.0f, 0.0f);
        makeFloat    (ParamIDs::filterEnvAmt,   "Filter Env Amt", -1.0f, 1.0f, 0.0f);
        makeFloat    (ParamIDs::filterKeyTrack, "Filter Key Track", 0.0f, 1.0f, 0.0f);

        // ============ Filter Envelope ============
        makeFloat (ParamIDs::filtAttack,  "Filt Attack",  0.001f, 5.0f, 0.005f, "s");
        makeFloat (ParamIDs::filtDecay,   "Filt Decay",   0.001f, 5.0f, 0.200f, "s");
        makeFloat (ParamIDs::filtSustain, "Filt Sustain", 0.0f,   1.0f, 0.500f);
        makeFloat (ParamIDs::filtRelease, "Filt Release", 0.001f, 10.0f, 0.300f, "s");

        // ============ Master ============
        makeFloat (ParamIDs::masterGain, "Master", 0.0f, 1.0f, 0.7f);

        // ============ FASE 4: LFOs ============
        // LFO 1
        makeChoice   (ParamIDs::lfo1Wave,  "LFO1 Wave",  lfoWaves, 0);
        makeFloatLog (ParamIDs::lfo1Rate,  "LFO1 Rate",  0.01f, 40.0f, 1.0f, "Hz");
        makeFloat    (ParamIDs::lfo1Depth, "LFO1 Depth", 0.0f, 1.0f, 0.0f);
        makeFloat    (ParamIDs::lfo1Phase, "LFO1 Phase", 0.0f, 1.0f, 0.0f);

        // LFO 2
        makeChoice   (ParamIDs::lfo2Wave,  "LFO2 Wave",  lfoWaves, 0);
        makeFloatLog (ParamIDs::lfo2Rate,  "LFO2 Rate",  0.01f, 40.0f, 1.0f, "Hz");
        makeFloat    (ParamIDs::lfo2Depth, "LFO2 Depth", 0.0f, 1.0f, 0.0f);
        makeFloat    (ParamIDs::lfo2Phase, "LFO2 Phase", 0.0f, 1.0f, 0.0f);

        // ============ FASE 4: Matriz de Modulación ============
        auto makeModSlot = [&] (const juce::String& srcId, const juce::String& dstId,
                                const juce::String& amtId, const juce::String& num)
        {
            makeChoice (srcId, "Mod" + num + " Src",  modSources, 0);
            makeChoice (dstId, "Mod" + num + " Dest", modDests,   0);
            makeFloat  (amtId, "Mod" + num + " Amt", -1.0f, 1.0f, 0.0f);
        };

        makeModSlot (ParamIDs::mod1Source, ParamIDs::mod1Dest, ParamIDs::mod1Amount, "1");
        makeModSlot (ParamIDs::mod2Source, ParamIDs::mod2Dest, ParamIDs::mod2Amount, "2");
        makeModSlot (ParamIDs::mod3Source, ParamIDs::mod3Dest, ParamIDs::mod3Amount, "3");
        makeModSlot (ParamIDs::mod4Source, ParamIDs::mod4Dest, ParamIDs::mod4Amount, "4");

        return { params.begin(), params.end() };
    }
}
