#pragma once
#include <JuceHeader.h>

class PresetManager
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& vts) : apvts(vts)
    {
        presetDirectory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
            .getChildFile("Textpure")
            .getChildFile("Presets");
            
        if (!presetDirectory.exists())
            presetDirectory.createDirectory();
            
        factoryPresets = { "Default", "Dark Clouds", "Digital Grit", "Ghost Melodies", "Subtle Texture", "Trap Shimmer" };
    }

    void savePreset(const juce::String& name)
    {
        if (name.isEmpty()) return;
        
        auto file = presetDirectory.getChildFile(name + ".xml");
        auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        xml->writeTo(file);
    }

    bool loadPreset(const juce::String& name)
    {
        // Try user presets first
        auto file = presetDirectory.getChildFile(name + ".xml");
        if (file.existsAsFile())
        {
            std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(file));
            if (xml != nullptr)
            {
                apvts.replaceState(juce::ValueTree::fromXml(*xml));
                return true;
            }
        }
        return false;
    }

    juce::StringArray getAllPresetNames()
    {
        juce::StringArray names = factoryPresets;
        
        juce::Array<juce::File> files;
        presetDirectory.findChildFiles(files, juce::File::findFiles, false, "*.xml");
        
        for (auto& f : files)
        {
            juce::String name = f.getFileNameWithoutExtension();
            if (!names.contains(name))
                names.add(name);
        }
            
        return names;
    }
    
    bool isFactoryPreset(const juce::String& name) const
    {
        return factoryPresets.contains(name);
    }

private:
    juce::AudioProcessorValueTreeState& apvts;
    juce::File presetDirectory;
    juce::StringArray factoryPresets;
};
