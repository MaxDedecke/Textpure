#include "ParticleSwarm.h"

ParticleSwarm::ParticleSwarm()
{
    particles.resize(maxParticles);
    for (auto& p : particles) {
        p.position = { (float)random.nextInt(800), (float)random.nextInt(600) };
        p.velocity = { (random.nextFloat() - 0.5f) * 2.0f, (random.nextFloat() - 0.5f) * 2.0f };
    }
    startTimerHz(60);
}

ParticleSwarm::~ParticleSwarm()
{
    stopTimer();
}

void ParticleSwarm::setParameters(float density, float texture, float size, float pitch, float level)
{
    activeParticles = (int)juce::jmap(density, 1.0f, 50.0f, 10.0f, (float)maxParticles);
    textureAmount = texture;
    sizeAmount = size / 500.0f; // Normalized size
    pitchAmount = pitch;
    audioLevel = level;
}

void ParticleSwarm::timerCallback()
{
    updateParticles();
    repaint();
}

void ParticleSwarm::updateParticles()
{
    auto bounds = getLocalBounds().toFloat();
    if (bounds.isEmpty()) return;

    float speedScale = 1.0f + (pitchAmount - 1.0f) * 0.5f;
    float jitter = textureAmount * 2.0f;

    for (int i = 0; i < activeParticles; ++i) {
        auto& p = particles[i];
        
        // Velocity update with texture jitter
        p.velocity += { (random.nextFloat() - 0.5f) * jitter, (random.nextFloat() - 0.5f) * jitter };
        
        // Keep velocity in check
        float maxVel = 3.0f * speedScale;
        p.velocity.setX(juce::jlimit(-maxVel, maxVel, p.velocity.getX()));
        p.velocity.setY(juce::jlimit(-maxVel, maxVel, p.velocity.getY()));

        p.position += p.velocity * speedScale;

        // Bounce off walls
        if (p.position.x < 0 || p.position.x > bounds.getWidth()) p.velocity.setX(p.velocity.getX() * -1.0f);
        if (p.position.y < 0 || p.position.y > bounds.getHeight()) p.velocity.setY(p.velocity.getY() * -1.0f);
        
        p.position.setX(juce::jlimit(0.0f, bounds.getWidth(), p.position.x));
        p.position.setY(juce::jlimit(0.0f, bounds.getHeight(), p.position.y));
    }
}

void ParticleSwarm::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float pulse = 1.0f + (audioLevel * 0.5f);
    
    // Blue interpolation for the whole swarm
    auto baseColor = juce::Colours::white.interpolatedWith(juce::Colours::blue, juce::jlimit(0.0f, 1.0f, audioLevel * 1.5f));
    
    float threshold = connectionThreshold * (0.5f + sizeAmount);

    for (int i = 0; i < activeParticles; ++i) {
        auto& p1 = particles[i];
        
        // Draw connections (edges)
        for (int j = i + 1; j < activeParticles; ++j) {
            auto& p2 = particles[j];
            float dist = p1.position.getDistanceFrom(p2.position);
            
            if (dist < threshold) {
                float alpha = juce::jmap(dist, 0.0f, threshold, 0.6f, 0.0f);
                g.setColour(baseColor.withAlpha(alpha)); 
                g.drawLine(p1.position.x, p1.position.y, p2.position.x, p2.position.y, 0.5f);
            }
        }
        
        // Draw particle
        g.setColour(baseColor);
        float s = p1.size * pulse;
        g.fillEllipse(p1.position.x - s/2, p1.position.y - s/2, s, s);
    }
}
