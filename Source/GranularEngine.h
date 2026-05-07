#pragma once
#include <JuceHeader.h>

/**
    Represents a single grain of audio in the granular synthesis engine.
*/
struct Grain
{
    float startSample = 0.0f;
    float currentSample = 0.0f;
    int durationSamples = 0;
    int ageSamples = 0;
    float pitch = 1.0f;
    bool isActive = false;

    void reset();
    float getSampleForChannel(const juce::AudioBuffer<float>& buffer, int channel, int bufferSize) const;
    void updateState(int bufferSize);
};

/**
    The core granular synthesis engine.
    Handles grain management, spawning, and processing.
*/
class GranularEngine
{
public:
    GranularEngine(int maxGrains = 40);

    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, 
                 float sizeMs, float density, float pitch, 
                 float texture, bool textureBypass);
    
    float getCurrentLevel() const { return lastLevel.load(); }

private:
    void spawnGrain(int writePos, float sizeMs, float pitch, float texture, bool textureBypass);
    
    std::vector<Grain> grains;
    juce::AudioBuffer<float> circularBuffer;
    int writePosition{ 0 };
    float samplesSinceLastGrain{ 0 };
    double currentSampleRate{ 44100.0 };
    
    std::atomic<float> lastLevel{ 0.0f };
    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GranularEngine)
};
