#pragma once

#include <JuceHeader.h>
#include "GranularEngine.h"
#include "PresetManager.h"

//==============================================================================
/**
    The main AudioProcessor for the TEXTPURE plugin.
    Acts as an orchestrator between the host, the granular engine, and effects.
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor,
                                 public juce::AudioProcessorValueTreeState::Listener
{
public:
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    // --- AudioProcessor Overrides ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // --- APVTS::Listener Overrides ---
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    // --- UI and State ---
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Public for Editor access
    juce::AudioProcessorValueTreeState apvts;
    PresetManager presetManager;
    
    float getCurrentLevel() const { return granularEngine.getCurrentLevel(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void updateReverbParameters(float reverbValue);
    void loadFactoryPreset(int index);

    GranularEngine granularEngine;
    
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    FrequencySplitter reverbFilter;

    std::atomic<bool> isUpdatingPresets{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)
};
