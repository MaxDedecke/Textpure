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
    A simple 3-band frequency splitter utility.
*/
class FrequencySplitter
{
public:
    void prepare(double sampleRate, int samplesPerBlock)
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, 2 };
        
        lowPass.prepare(spec);
        lowPass.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        lowPass.setCutoffFrequency(250.0f);

        highPass.prepare(spec);
        highPass.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        highPass.setCutoffFrequency(2500.0f);

        midLP.prepare(spec);
        midLP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        midLP.setCutoffFrequency(2500.0f);

        midHP.prepare(spec);
        midHP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        midHP.setCutoffFrequency(250.0f);
    }

    void process(juce::AudioBuffer<float>& buffer, int band)
    {
        if (band == 0) return; // Full

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        if (band == 1) // Low
        {
            lowPass.process(context);
        }
        else if (band == 2) // Mid
        {
            midLP.process(context);
            midHP.process(context);
        }
        else if (band == 3) // High
        {
            highPass.process(context);
        }
    }

private:
    juce::dsp::LinkwitzRileyFilter<float> lowPass, highPass, midLP, midHP;
};

/**
    The core granular synthesis engine.
    Handles grain management, spawning, and processing.
*/
class GranularEngine
{
public:
    GranularEngine(int maxGrains = 128); // Increased for smoother stretching

    void prepare(double sampleRate, int maxBlockSize);
    void process(juce::AudioBuffer<float>& buffer, 
                 float sizeMs, float density, float pitch, 
                 float texture, bool textureBypass,
                 bool syncEnabled, int rateIndex, 
                 bool sizeSyncEnabled, int sizeRateIndex,
                 int granularBand, int textureBand,
                 double bpm);
    
    float getCurrentLevel() const { return lastLevel.load(); }
    int getLatencySamples() const { return (int)(0.150 * currentSampleRate); } // Report 150ms stable window

private:
    void spawnGrain(int writePos, float sizeMs, float pitch, float texture, bool textureBypass);
    
    std::vector<Grain> grains;
    juce::AudioBuffer<float> circularBuffer;
    int writePosition{ 0 };
    float samplesSinceLastGrain{ 0 };
    double currentSampleRate{ 44100.0 };
    
    juce::LinearSmoothedValue<float> smoothedPitch;

    std::atomic<float> lastLevel{ 0.0f };
    juce::Random random;

    FrequencySplitter granularFilter;
    FrequencySplitter textureFilter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GranularEngine)
};
