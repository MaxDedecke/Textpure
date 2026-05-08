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

    granularFilter.prepare(sampleRate, maxBlockSize);
    textureFilter.prepare(sampleRate, maxBlockSize);

    smoothedPitch.reset(sampleRate, 0.05);
    smoothedPitch.setCurrentAndTargetValue(1.0f);
}

void GranularEngine::process(juce::AudioBuffer<float>& buffer, 
                             float sizeMs, float density, float pitch, 
                             float texture, bool textureBypass,
                             bool syncEnabled, int rateIndex, 
                             bool sizeSyncEnabled, int sizeRateIndex,
                             int granularBand, int textureBand,
                             double bpm)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    const int circularBufferSize = circularBuffer.getNumSamples();
    
    smoothedPitch.setTargetValue(pitch);

    // --- 1. Calculate Intervals ---
    float grainIntervalSamples;
    float divisions[] = { 
        1.0f, 2.0f/3.0f, 1.5f,              // 1/4
        0.5f, 1.0f/3.0f, 0.75f,             // 1/8
        0.25f, 0.5f/3.0f, 0.375f,           // 1/16
        0.125f, 0.0625f, 0.03125f           // 1/32, 1/64, 1/128
    };

    if (syncEnabled && bpm > 0)
    {
        float division = divisions[juce::jlimit(0, 11, rateIndex)];
        float secondsPerBeat = 60.0f / (float)bpm;
        grainIntervalSamples = secondsPerBeat * division * (float)currentSampleRate;
    }
    else
    {
        grainIntervalSamples = (float)currentSampleRate / density;
    }

    float effectiveSizeMs = sizeMs;
    if (sizeSyncEnabled && bpm > 0)
    {
        float division = divisions[juce::jlimit(0, 11, sizeRateIndex)];
        float secondsPerBeat = 60.0f / (float)bpm;
        effectiveSizeMs = secondsPerBeat * division * 1000.0f;
    }

    // --- 2. Record Input and Process Grains ---
    float totalLevel = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float inL = (numChannels > 0) ? buffer.getSample(0, sample) : 0.0f;
        float inR = (numChannels > 1) ? buffer.getSample(1, sample) : inL;

        circularBuffer.setSample(0, writePosition, inL);
        circularBuffer.setSample(1, writePosition, inR);
        totalLevel += std::abs(inL);

        float currentPitch = smoothedPitch.getNextValue();

        // Spawning
        samplesSinceLastGrain++;
        if (samplesSinceLastGrain >= grainIntervalSamples)
        {
            samplesSinceLastGrain = 0;
            spawnGrain(writePosition, effectiveSizeMs, currentPitch, texture, textureBypass);
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
            // Square-root normalization for energy conservation
            float norm = 1.0f / std::sqrt((float)activeCount);
            grainOutL *= norm;
            grainOutR *= norm;
        }

        buffer.setSample(0, sample, grainOutL);
        if (numChannels > 1) buffer.setSample(1, sample, grainOutR);

        writePosition = (writePosition + 1) % circularBufferSize;
    }

    lastLevel.store(totalLevel / (float)numSamples);

    // --- 3. Apply Granular Band Filtering ---
    if (granularBand != 0)
    {
        granularFilter.process(buffer, granularBand);
    }

    // --- 4. Apply Texture (Saturation) with Band Restriction ---
    if (!textureBypass && texture > 0.05f)
    {
        if (textureBand == 0) // Full Spectrum
        {
            float drive = 1.0f + texture * 2.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                auto* ptr = buffer.getWritePointer(ch);
                for (int s = 0; s < buffer.getNumSamples(); ++s)
                    ptr[s] = std::tanh(ptr[s] * drive);
            }
        }
        else // Restricted Band
        {
            juce::AudioBuffer<float> bandBuffer;
            bandBuffer.makeCopyOf(buffer);
            textureFilter.process(bandBuffer, textureBand);

            juce::AudioBuffer<float> unprocessedBand;
            unprocessedBand.makeCopyOf(buffer);
            textureFilter.process(unprocessedBand, textureBand);

            float drive = 1.0f + texture * 2.0f;
            for (int ch = 0; ch < bandBuffer.getNumChannels(); ++ch)
            {
                auto* ptr = bandBuffer.getWritePointer(ch);
                auto* cleanPtr = buffer.getWritePointer(ch);
                auto* unprocPtr = unprocessedBand.getReadPointer(ch);
                
                for (int s = 0; s < bandBuffer.getNumSamples(); ++s)
                {
                    float saturated = std::tanh(ptr[s] * drive);
                    cleanPtr[s] = cleanPtr[s] - unprocPtr[s] + saturated;
                }
            }
        }
    }
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

            // --- STRETCH MODE LOGIC ---
            // To prevent temporal shifts when pitching, we pull audio from a stable window 
            // behind the write head. This constant offset (e.g. 150ms) ensures that transients 
            // always appear at the correct time relative to the input.
            
            float stableDelaySamples = (float)currentSampleRate * 0.150f; // 150ms fixed delay
            
            // We must ensure the grain doesn't read into the "future" (ahead of writePos)
            // if it plays back faster (pitch > 1.0) or starts too early.
            float requiredSafety = (float)g.durationSamples * pitch;
            float actualOffset = juce::jmax(stableDelaySamples, requiredSafety + 100.0f);

            float jitter = textureBypass ? 0.0f : (random.nextFloat() - 0.5f) * texture * 5000.0f;
            g.startSample = (float)writePos - actualOffset - jitter;

            int bufSize = circularBuffer.getNumSamples();
            while (g.startSample < 0) g.startSample += (float)bufSize;
            g.currentSample = g.startSample;
            break;
        }
    }
}
