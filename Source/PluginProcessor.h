/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
struct Grain
{
    float startSample = 0;
    float currentSample = 0;
    int durationSamples = 0;
    int ageSamples = 0;
    float pitch = 1.0f;
    bool isActive = false;

    void reset() { isActive = false; ageSamples = 0; }

    // Gibt das Sample fuer einen Kanal zurueck OHNE den Status zu veraendern
    float getSampleForChannel(const juce::AudioBuffer<float>& buffer, int channel, int bufferSize) const
    {
        if (!isActive || bufferSize <= 0) return 0.0f;
        
        float pos = currentSample;
        while (pos < 0) pos += (float)bufferSize;
        while (pos >= (float)bufferSize) pos -= (float)bufferSize;

        int index1 = (int)pos;
        int index2 = (index1 + 1) % bufferSize;
        float fraction = pos - (float)index1;
        
        float sample1 = buffer.getSample(channel, index1);
        float sample2 = buffer.getSample(channel, index2);
        float sample = sample1 + fraction * (sample2 - sample1);
        
        float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * (float)ageSamples / (float)durationSamples));
        return sample * window;
    }

    // Aktualisiert den Status des Grains (einmal pro Sample-Zeitpunkt aufrufen)
    void updateState(int bufferSize)
    {
        if (!isActive) return;
        currentSample += pitch;
        if (currentSample >= (float)bufferSize) currentSample -= (float)bufferSize;
        ageSamples++;
        if (ageSamples >= durationSamples) isActive = false;
    }
};

//==============================================================================
class NewProjectAudioProcessor  : public juce::AudioProcessor
{
public:
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> currentLevel{ 0.0f };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    static constexpr int maxGrains = 40;
    std::vector<Grain> grains;
    juce::AudioBuffer<float> circularBuffer;
    int writePosition{ 0 };
    float samplesSinceLastGrain{ 0 };

    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)
};
