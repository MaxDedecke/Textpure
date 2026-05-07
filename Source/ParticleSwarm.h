#pragma once
#include <JuceHeader.h>
#include <vector>

class ParticleSwarm : public juce::Component, public juce::Timer
{
public:
    ParticleSwarm();
    ~ParticleSwarm() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

    void setParameters(float density, float texture, float size, float pitch, float level);

private:
    struct Particle {
        juce::Point<float> position;
        juce::Point<float> velocity;
        float size = 2.0f;
    };

    void updateParticles();
    
    std::vector<Particle> particles;
    int maxParticles = 100;
    int activeParticles = 20;
    
    float textureAmount = 0.0f;
    float sizeAmount = 0.5f;
    float audioLevel = 0.0f;
    float pitchAmount = 1.0f;
    
    juce::Random random;
    const float connectionThreshold = 60.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParticleSwarm)
};
