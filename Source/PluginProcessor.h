/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GranularEngine.h"

//==============================================================================
/**
    The main AudioProcessor for the TEXTPURE plugin.
    Acts as an orchestrator between the host, the granular engine, and effects.
*/
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    // --- AudioProcessor Overrides ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    float getCurrentLevel() const { return granularEngine.getCurrentLevel(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void updateReverbParameters(float reverbValue);

    GranularEngine granularEngine;

    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)
};

