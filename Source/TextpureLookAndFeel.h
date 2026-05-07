#pragma once
#include <JuceHeader.h>

class TextpureLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TextpureLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
        setColour(juce::Slider::trackColourId, juce::Colour(0xFF00E5FF)); // Turquoise
        setColour(juce::TextButton::buttonColourId, juce::Colours::black);
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float startAngle, const float endAngle, juce::Slider& slider) override
    {
        auto outline = juce::Colours::white.withAlpha(0.2f);
        auto fill = juce::Colour(0xFF00E5FF);

        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = startAngle + sliderPos * (endAngle - startAngle);
        auto lineW = 2.0f;
        auto arcRadius = radius - lineW * 0.5f;

        juce::Path backgroundArc;
        backgroundArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, startAngle, endAngle, true);
        g.setColour(outline);
        g.strokePath(backgroundArc, juce::PathStrokeType(lineW));

        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc(bounds.getCentreX(), bounds.getCentreY(), arcRadius, arcRadius, 0.0f, startAngle, toAngle, true);
            g.setColour(fill);
            g.strokePath(valueArc, juce::PathStrokeType(lineW));
        }
        
        // Draw parameter name in center
        g.setColour(juce::Colours::white);
        g.setFont(12.0f);
        g.drawText(slider.getName(), x, y + height - 15, width, 15, juce::Justification::centred);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(2);
        auto fill = button.getToggleState() ? juce::Colour(0xFF00E5FF) : juce::Colours::white.withAlpha(0.1f);
        
        g.setColour(fill);
        g.drawRect(bounds, 1.0f);
        
        if (button.getToggleState()) {
            g.fillRect(bounds.reduced(3));
        }
    }
};
