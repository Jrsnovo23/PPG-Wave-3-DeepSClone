#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace presets
{
    struct PresetInfo
    {
        juce::String name;
        juce::String category;
        juce::String author;
        bool         isFactory = false;
        juce::File   file;      // vacío si es factory
    };

    class PresetManager
    {
    public:
        explicit PresetManager (juce::AudioProcessorValueTreeState& apvts);

        const juce::Array<PresetInfo>& getAllPresets() const { return allPresets; }

        void loadByIndex (int index);
        void loadFromFile (const juce::File& f);

        void next();
        void prev();

        bool saveUserPreset (const juce::String& name, const juce::String& category);

        juce::String getCurrentName() const;
        juce::String getCurrentCategory() const;
        int          getCurrentIndex() const { return currentIndex; }

        void refresh();

        juce::File getUserPresetDirectory() const;

    private:
        void applyPreset (const juce::var& preset);
        juce::var captureCurrentState() const;
        void loadFactoryPresets();

        juce::AudioProcessorValueTreeState& apvts;
        juce::Array<PresetInfo> allPresets;
        int currentIndex = 0;
    };
}
