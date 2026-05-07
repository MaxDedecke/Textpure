#pragma once
#include <JuceHeader.h>

class TextpureLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TextpureLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
        setColour(juce::Slider::trackColourId, juce::Colour(0xFF00E5FF)); // Turquoise
        
        setColour(juce::ComboBox::backgroundColourId, juce::Colours::black);
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::white.withAlpha(0.5f));
        setColour(juce::ComboBox::arrowColourId, juce::Colour(0xFF00E5FF));
        
        setColour(juce::PopupMenu::backgroundColourId, juce::Colours::black);
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xFF00E5FF));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
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

        // Draw parameter name in center using Impact font
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font("Impact", 14.0f, juce::Font::plain));
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

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonWidth, int buttonHeight, juce::ComboBox& box) override
    {
        auto cornerSize = 0.0f;
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

        g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
        g.fillRect(bounds);

        g.setColour(box.findColour(juce::ComboBox::outlineColourId));
        g.drawRect(bounds, 1.0f);

        juce::Path path;
        path.addTriangle((float)width - 15.0f, (float)height * 0.5f - 2.0f,
                         (float)width - 5.0f, (float)height * 0.5f - 2.0f,
                         (float)width - 10.0f, (float)height * 0.5f + 3.0f);

        g.setColour(box.findColour(juce::ComboBox::arrowColourId));
        g.fillPath(path);
    }

    juce::Font getComboBoxFont(juce::ComboBox& box) override
    {
        return juce::Font("Impact", 16.0f, juce::Font::plain);
    }

    juce::Font getPopupMenuFont() override
    {
        return juce::Font("Impact", 16.0f, juce::Font::plain);
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colour(0xFF00E5FF));
        g.drawRect(0, 0, width, height, 1);
    }
};
