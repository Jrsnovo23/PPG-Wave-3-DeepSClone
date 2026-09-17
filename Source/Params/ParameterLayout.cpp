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

        const juce::StringArray waveChoices { "Sine", "Triangle", "Saw", "Square" };
        const juce::StringArray octaveChoices { "-2", "-1", "0", "+1", "+2" };

        makeChoice (ParamIDs::osc1Wave,   "Osc1 Wave",   waveChoices,   0);
        makeFloat  (ParamIDs::osc1Pos,    "Osc1 Pos",    0.0f, 1.0f, 0.0f);
        makeChoice (ParamIDs::osc1Octave, "Osc1 Octave", octaveChoices, 2);
        makeFloat  (ParamIDs::osc1Semi,   "Osc1 Semi",  -12.0f, 12.0f, 0.0f, "st");
        makeFloat  (ParamIDs::osc1Fine,   "Osc1 Fine", -100.0f, 100.0f, 0.0f, "ct");
        makeFloat  (ParamIDs::osc1Level,  "Osc1 Level",  0.0f, 1.0f, 0.8f);

        makeChoice (ParamIDs::osc2Wave,   "Osc2 Wave",   waveChoices,   2);
        makeFloat  (ParamIDs::osc2Pos,    "Osc2 Pos",    0.0f, 1.0f, 0.0f);
        makeChoice (ParamIDs::osc2Octave, "Osc2 Octave", octaveChoices, 2);
        makeFloat  (ParamIDs::osc2Semi,   "Osc2 Semi",  -12.0f, 12.0f, 0.0f, "st");
        makeFloat  (ParamIDs::osc2Fine,   "Osc2 Fine", -100.0f, 100.0f, 0.0f, "ct");
        makeFloat  (ParamIDs::osc2Level,  "Osc2 Level",  0.0f, 1.0f, 0.0f);

        makeFloat  (ParamIDs::ampAttack,  "Amp Attack",  0.001f, 5.0f, 0.005f, "s");
        makeFloat  (ParamIDs::ampDecay,   "Amp Decay",   0.001f, 5.0f, 0.100f, "s");
        makeFloat  (ParamIDs::ampSustain, "Amp Sustain", 0.0f,   1.0f, 0.700f);
        makeFloat  (ParamIDs::ampRelease, "Amp Release", 0.001f, 10.0f, 0.300f, "s");

        makeFloat  (ParamIDs::masterGain, "Master",      0.0f, 1.0f, 0.7f);

        return { params.begin(), params.end() };
    }
}
