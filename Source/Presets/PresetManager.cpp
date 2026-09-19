#include "PresetManager.h"

namespace presets
{
    // ============ Factory presets (JSON) ============
    static const char* kFactoryJSON[] =
    {
        // 1 — Init Saw
        R"({"name":"Init Saw","category":"Init","author":"Factory","params":{
            "osc1_wave":2,"osc1_pos":0.0,"osc1_octave":2,"osc1_level":0.8,
            "osc2_wave":0,"osc2_level":0.0,
            "filter_type":0,"filter_cutoff":8000,"filter_reso":0.0,"filter_env_amt":0.0,
            "amp_attack":0.005,"amp_decay":0.1,"amp_sustain":0.7,"amp_release":0.3,
            "filt_attack":0.005,"filt_decay":0.2,"filt_sustain":0.5,"filt_release":0.3,
            "master_gain":0.7
        }})",

        // 2 — Digital Pad
        R"({"name":"Digital Pad","category":"Pad","author":"Factory","params":{
            "osc1_wave":2,"osc1_pos":0.3,"osc1_level":0.6,
            "osc2_wave":3,"osc2_pos":0.5,"osc2_semi":7,"osc2_level":0.4,
            "filter_type":0,"filter_cutoff":3500,"filter_reso":0.2,
            "amp_attack":0.8,"amp_decay":1.5,"amp_sustain":0.75,"amp_release":2.0,
            "filt_attack":0.6,"filt_decay":1.2,"filt_sustain":0.5,"filt_release":1.5,
            "chorus_on":1,"chorus_rate":0.3,"chorus_depth":0.4,"chorus_mix":0.5,
            "reverb_on":1,"reverb_size":0.7,"reverb_damp":0.5,"reverb_mix":0.4,
            "master_gain":0.6
        }})",

        // 3 — Metallic Bell
        R"({"name":"Metallic Bell","category":"Bell","author":"Factory","params":{
            "osc1_wave":3,"osc1_pos":0.7,"osc1_level":0.8,
            "osc2_wave":3,"osc2_pos":0.2,"osc2_semi":19,"osc2_level":0.5,
            "filter_type":0,"filter_cutoff":6000,"filter_reso":0.65,"filter_env_amt":-0.3,
            "amp_attack":0.001,"amp_decay":1.8,"amp_sustain":0.0,"amp_release":1.5,
            "filt_attack":0.001,"filt_decay":0.6,"filt_sustain":0.0,"filt_release":0.5,
            "reverb_on":1,"reverb_size":0.5,"reverb_damp":0.4,"reverb_mix":0.35,
            "master_gain":0.55
        }})",

        // 4 — Bass Pulse
        R"({"name":"Bass Pulse","category":"Bass","author":"Factory","params":{
            "osc1_wave":3,"osc1_octave":1,"osc1_level":0.9,
            "osc2_wave":2,"osc2_octave":1,"osc2_semi":-12,"osc2_level":0.4,
            "filter_type":0,"filter_cutoff":900,"filter_reso":0.45,"filter_env_amt":0.5,
            "amp_attack":0.001,"amp_decay":0.3,"amp_sustain":0.6,"amp_release":0.15,
            "filt_attack":0.001,"filt_decay":0.25,"filt_sustain":0.2,"filt_release":0.15,
            "drive_on":1,"drive_amount":4.0,"drive_tone":0.4,"drive_mix":0.4,
            "master_gain":0.65
        }})",

        // 5 — Crystal Lead
        R"({"name":"Crystal Lead","category":"Lead","author":"Factory","params":{
            "osc1_wave":2,"osc1_level":0.7,
            "osc2_wave":2,"osc2_semi":12,"osc2_fine":8.0,"osc2_level":0.6,
            "filter_type":0,"filter_cutoff":9500,"filter_reso":0.3,
            "amp_attack":0.01,"amp_decay":0.2,"amp_sustain":0.85,"amp_release":0.4,
            "delay_on":1,"delay_sync":4,"delay_feedback":0.35,"delay_mix":0.3,
            "reverb_on":1,"reverb_size":0.4,"reverb_damp":0.6,"reverb_mix":0.25,
            "master_gain":0.6
        }})",

        // 6 — Evolving Sweep
        R"({"name":"Evolving Sweep","category":"Pad","author":"Factory","params":{
            "osc1_wave":2,"osc1_pos":0.0,"osc1_level":0.7,
            "osc2_wave":3,"osc2_pos":0.0,"osc2_semi":-5,"osc2_level":0.5,
            "filter_type":0,"filter_cutoff":1200,"filter_reso":0.55,
            "amp_attack":1.2,"amp_decay":2.0,"amp_sustain":0.8,"amp_release":2.5,
            "lfo1_wave":0,"lfo1_rate":0.15,"lfo1_depth":0.9,"lfo1_phase":0.0,
            "mod1_source":1,"mod1_dest":5,"mod1_amount":0.7,
            "mod2_source":2,"mod2_dest":4,"mod2_amount":0.5,
            "reverb_on":1,"reverb_size":0.85,"reverb_damp":0.3,"reverb_mix":0.45,
            "master_gain":0.55
        }})",

        // 7 — Pluck Attack
        R"({"name":"Pluck Attack","category":"Pluck","author":"Factory","params":{
            "osc1_wave":2,"osc1_level":0.8,
            "osc2_wave":3,"osc2_octave":3,"osc2_level":0.3,
            "filter_type":0,"filter_cutoff":7000,"filter_reso":0.4,"filter_env_amt":0.7,
            "amp_attack":0.001,"amp_decay":0.15,"amp_sustain":0.0,"amp_release":0.2,
            "filt_attack":0.001,"filt_decay":0.12,"filt_sustain":0.0,"filt_release":0.15,
            "delay_on":1,"delay_sync":5,"delay_feedback":0.4,"delay_mix":0.25,
            "master_gain":0.65
        }})",

        // 8 — Vintage Wave
        R"({"name":"Vintage Wave","category":"Lead","author":"Factory","params":{
            "osc1_wave":2,"osc1_level":0.7,"osc1_fine":-4.0,
            "osc2_wave":2,"osc2_level":0.7,"osc2_fine":5.0,
            "filter_type":0,"filter_cutoff":5500,"filter_reso":0.25,
            "amp_attack":0.02,"amp_decay":0.3,"amp_sustain":0.8,"amp_release":0.5,
            "chorus_on":1,"chorus_rate":0.6,"chorus_depth":0.5,"chorus_mix":0.4,
            "master_gain":0.6
        }})",

        // 9 — Sub Drone
        R"({"name":"Sub Drone","category":"Atmospheric","author":"Factory","params":{
            "osc1_wave":0,"osc1_octave":0,"osc1_level":0.9,
            "osc2_wave":2,"osc2_octave":0,"osc2_semi":0,"osc2_level":0.3,
            "filter_type":0,"filter_cutoff":600,"filter_reso":0.15,
            "amp_attack":1.5,"amp_decay":2.0,"amp_sustain":0.85,"amp_release":3.0,
            "lfo1_wave":0,"lfo1_rate":0.08,"lfo1_depth":0.4,
            "mod1_source":1,"mod1_dest":5,"mod1_amount":0.3,
            "reverb_on":1,"reverb_size":0.9,"reverb_damp":0.2,"reverb_mix":0.5,
            "master_gain":0.5
        }})",

        // 10 — Digital FX
        R"({"name":"Digital FX","category":"FX","author":"Factory","params":{
            "osc1_wave":3,"osc1_pos":0.5,"osc1_level":0.6,
            "osc2_wave":3,"osc2_pos":0.8,"osc2_semi":7,"osc2_level":0.5,
            "filter_type":2,"filter_cutoff":2000,"filter_reso":0.7,
            "amp_attack":0.05,"amp_decay":0.5,"amp_sustain":0.5,"amp_release":1.5,
            "lfo1_wave":5,"lfo1_rate":8.0,"lfo1_depth":0.7,
            "mod1_source":1,"mod1_dest":3,"mod1_amount":0.6,
            "mod2_source":1,"mod2_dest":5,"mod2_amount":0.5,
            "delay_on":1,"delay_sync":6,"delay_feedback":0.6,"delay_mix":0.4,
            "reverb_on":1,"reverb_size":0.6,"reverb_damp":0.3,"reverb_mix":0.3,
            "master_gain":0.5
        }})"
    };

    // ============ PresetManager ============

    PresetManager::PresetManager (juce::AudioProcessorValueTreeState& a) : apvts (a)
    {
        loadFactoryPresets();
        refresh();
    }

    void PresetManager::loadFactoryPresets()
    {
        for (auto* jsonStr : kFactoryJSON)
        {
            auto parsed = juce::JSON::parse (juce::String (jsonStr));
            if (auto* obj = parsed.getDynamicObject())
            {
                PresetInfo info;
                info.name     = obj->getProperty ("name").toString();
                info.category = obj->getProperty ("category").toString();
                info.author   = obj->getProperty ("author").toString();
                info.isFactory = true;
                allPresets.add (info);
            }
        }
    }

    juce::File PresetManager::getUserPresetDirectory() const
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("OpenWave Audio")
                       .getChildFile ("PPGWave3Clone")
                       .getChildFile ("Presets");
        if (! dir.exists())
            dir.createDirectory();
        return dir;
    }

    void PresetManager::refresh()
    {
        // Conserva solo los factory
        juce::Array<PresetInfo> factoryOnly;
        for (const auto& p : allPresets)
            if (p.isFactory) factoryOnly.add (p);

        allPresets = factoryOnly;

        // Añade los del usuario
        auto dir = getUserPresetDirectory();
        juce::Array<juce::File> files;
        dir.findChildFiles (files, juce::File::findFiles, false, "*.json");

        for (const auto& f : files)
        {
            auto text = f.loadFileAsString();
            auto parsed = juce::JSON::parse (text);
            if (auto* obj = parsed.getDynamicObject())
            {
                PresetInfo info;
                info.name     = obj->getProperty ("name").toString();
                info.category = obj->getProperty ("category").toString();
                info.author   = obj->getProperty ("author").toString();
                info.isFactory = false;
                info.file      = f;
                allPresets.add (info);
            }
        }
    }

    void PresetManager::applyPreset (const juce::var& preset)
    {
        auto* obj = preset.getDynamicObject();
        if (! obj) return;

        auto paramsVar = obj->getProperty ("params");
        auto* paramsObj = paramsVar.getDynamicObject();
        if (! paramsObj) return;

        for (const auto& prop : paramsObj->getProperties())
        {
            const auto id = prop.name.toString();
            if (auto* param = apvts.getParameter (id))
            {
                const float targetValue = (float) prop.value;
                const auto& range = param->getNormalisableRange();
                const float normalized = range.convertTo0to1 (targetValue);
                param->setValueNotifyingHost (normalized);
            }
        }
    }

    juce::var PresetManager::captureCurrentState() const
    {
        auto* root = new juce::DynamicObject();
        auto* params = new juce::DynamicObject();

        for (auto* p : apvts.processor.getParameters())
        {
            if (auto* withId = dynamic_cast<juce::AudioProcessorParameterWithID*> (p))
            {
                const float norm = p->getValue();
                const float real = p->getNormalisableRange().convertFrom0to1 (norm);
                params->setProperty (withId->paramID, real);
            }
        }

        root->setProperty ("params", juce::var (params));
        return juce::var (root);
    }

    void PresetManager::loadByIndex (int index)
    {
        if (allPresets.isEmpty()) return;
        index = juce::jlimit (0, allPresets.size() - 1, index);
        currentIndex = index;

        const auto& info = allPresets.getReference (index);
        juce::var preset;

        if (info.isFactory)
        {
            // Re-parsear el JSON del factory
            int i = 0;
            for (auto* jsonStr : kFactoryJSON)
            {
                if (i == index)
                {
                    preset = juce::JSON::parse (juce::String (jsonStr));
                    break;
                }
                ++i;
            }
        }
        else
        {
            preset = juce::JSON::parse (info.file.loadFileAsString());
        }

        if (! preset.isVoid())
            applyPreset (preset);
    }

    void PresetManager::loadFromFile (const juce::File& f)
    {
        if (! f.existsAsFile()) return;
        auto preset = juce::JSON::parse (f.loadFileAsString());
        if (! preset.isVoid())
            applyPreset (preset);
    }

    void PresetManager::next()
    {
        if (allPresets.isEmpty()) return;
        loadByIndex ((currentIndex + 1) % allPresets.size());
    }

    void PresetManager::prev()
    {
        if (allPresets.isEmpty()) return;
        int i = currentIndex - 1;
        if (i < 0) i = allPresets.size() - 1;
        loadByIndex (i);
    }

    bool PresetManager::saveUserPreset (const juce::String& name, const juce::String& category)
    {
        if (name.isEmpty()) return false;

        auto root = captureCurrentState();
        auto* obj = root.getDynamicObject();
        if (! obj) return false;

        obj->setProperty ("name", name);
        obj->setProperty ("category", category);
        obj->setProperty ("author", "User");

        auto dir = getUserPresetDirectory();
        auto safeName = juce::File::createLegalFileName (name);
        auto file = dir.getChildFile (safeName + ".json");

        if (! file.replaceWithText (juce::JSON::toString (root, true)))
            return false;

        refresh();
        return true;
    }

    juce::String PresetManager::getCurrentName() const
    {
        if (allPresets.isEmpty()) return "—";
        return allPresets.getReference (currentIndex).name;
    }

    juce::String PresetManager::getCurrentCategory() const
    {
        if (allPresets.isEmpty()) return "—";
        return allPresets.getReference (currentIndex).category;
    }
}
