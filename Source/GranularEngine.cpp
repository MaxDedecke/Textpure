#include "GranularEngine.h"

// --- Grain Implementation ---

void Grain::reset() { 
    isActive = false; 
    ageSamples = 0; 
}

float Grain::getSampleForChannel(const juce::AudioBuffer<float>& buffer, int channel, int bufferSize) const
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

    // Hann Window
    float window = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * (float)ageSamples / (float)durationSamples));
    return sample * window;
}

void Grain::updateState(int bufferSize)
{
    if (!isActive) return;
    currentSample += pitch;
    if (currentSample >= (float)bufferSize) currentSample -= (float)bufferSize;
    
    ageSamples++;
    if (ageSamples >= durationSamples) isActive = false;
}

// --- GranularEngine Implementation ---

GranularEngine::GranularEngine(int maxGrains)
{
    grains.resize(maxGrains);
}

void GranularEngine::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    circularBuffer.setSize(2, (int)(sampleRate * 2.0)); // 2 Seconds
    circularBuffer.clear();
    writePosition = 0;
    samplesSinceLastGrain = 0;
    for (auto& g : grains) g.reset();
}

void GranularEngine::process(juce::AudioBuffer<float>& buffer, 
                             float sizeMs, float density, float pitch, 
                             float texture, bool textureBypass,
                             bool syncEnabled, int rateIndex, double bpm)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    const int circularBufferSize = circularBuffer.getNumSamples();
    
    float grainIntervalSamples;
    
    if (syncEnabled && bpm > 0)
    {
        // 0=1/4, 1=1/4T, 2=1/4D, 3=1/8, 4=1/8T, 5=1/8D, 6=1/16, 7=1/16T, 8=1/16D, 9=1/32, 10=1/64, 11=1/128
        float divisions[] = { 
            1.0f, 2.0f/3.0f, 1.5f,              // 1/4
            0.5f, 1.0f/3.0f, 0.75f,             // 1/8
            0.25f, 0.5f/3.0f, 0.375f,           // 1/16
            0.125f, 0.0625f, 0.03125f           // 1/32, 1/64, 1/128
        };
        float division = divisions[juce::jlimit(0, 11, rateIndex)];
        
        float secondsPerBeat = 60.0f / (float)bpm;
        float syncIntervalSeconds = secondsPerBeat * division;
        grainIntervalSamples = syncIntervalSeconds * (float)currentSampleRate;
    }
    else
    {
        grainIntervalSamples = (float)currentSampleRate / density;
    }

    float totalLevel = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float inL = (numChannels > 0) ? buffer.getSample(0, sample) : 0.0f;
        float inR = (numChannels > 1) ? buffer.getSample(1, sample) : inL;

        circularBuffer.setSample(0, writePosition, inL);
        circularBuffer.setSample(1, writePosition, inR);
        totalLevel += std::abs(inL);

        // Spawning
        samplesSinceLastGrain++;
        if (samplesSinceLastGrain >= grainIntervalSamples)
        {
            samplesSinceLastGrain = 0;
            spawnGrain(writePosition, sizeMs, pitch, texture, textureBypass);
        }

        // Processing
        float grainOutL = 0, grainOutR = 0;
        int activeCount = 0;
        
        for (auto& g : grains)
        {
            if (g.isActive)
            {
                grainOutL += g.getSampleForChannel(circularBuffer, 0, circularBufferSize);
                grainOutR += g.getSampleForChannel(circularBuffer, 1, circularBufferSize);
                g.updateState(circularBufferSize);
                activeCount++;
            }
        }

        if (activeCount > 0)
        {
            float norm = 1.0f / std::sqrt((float)activeCount);
            grainOutL *= norm;
            grainOutR *= norm;
        }

        // Apply Texture (Saturation)
        if (!textureBypass && texture > 0.05f)
        {
            float drive = 1.0f + texture * 2.0f;
            grainOutL = std::tanh(grainOutL * drive);
            grainOutR = std::tanh(grainOutR * drive);
        }

        // We only return the wet granular signal in this buffer
        // The PluginProcessor will handle the dry/wet mix
        buffer.setSample(0, sample, grainOutL);
        if (numChannels > 1) buffer.setSample(1, sample, grainOutR);

        writePosition = (writePosition + 1) % circularBufferSize;
    }

    lastLevel.store(totalLevel / (float)numSamples);
}

void GranularEngine::spawnGrain(int writePos, float sizeMs, float pitch, float texture, bool textureBypass)
{
    for (auto& g : grains)
    {
        if (!g.isActive)
        {
            g.isActive = true;
            g.durationSamples = juce::jmax(10, (int)(currentSampleRate * (sizeMs / 1000.0f)));
            g.ageSamples = 0;
            g.pitch = pitch;

            float jitter = textureBypass ? 0.0f : (random.nextFloat() - 0.5f) * texture * 5000.0f;
            g.startSample = (float)writePos - (g.durationSamples * g.pitch) - 500.0f - jitter;

            int bufSize = circularBuffer.getNumSamples();
            while (g.startSample < 0) g.startSample += (float)bufSize;
            g.currentSample = g.startSample;
            break;
        }
    }
}
