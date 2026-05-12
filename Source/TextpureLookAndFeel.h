#pragma once
#include <JuceHeader.h>

class TextpureLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TextpureLookAndFeel()
    {
        setColour(juce::Slider::thumbColourId, juce::Colours::white);
        setColour(juce::Slider::trackColourId, juce::Colours::white); 
        
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF0A0A0A));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::outlineColourId, juce::Colours::white.withAlpha(0.1f));
        setColour(juce::ComboBox::arrowColourId, juce::Colours::white.withAlpha(0.6f));
        
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF0A0A0A));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::white);
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
    }

    void setAudioLevel(float level) { audioLevel = level; }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float startAngle, const float endAngle, juce::Slider& slider) override
    {
        float pulse = 0.1f + (audioLevel * 0.4f);
        
        // Blue interpolation
        auto baseColor = juce::Colours::white.interpolatedWith(juce::Colours::blue, juce::jlimit(0.0f, 1.0f, audioLevel * 1.5f));
        
        auto outline = baseColor.withAlpha(slider.isEnabled() ? (0.1f + pulse) : 0.05f);
        auto fill = slider.isEnabled() ? baseColor.withAlpha(0.6f + pulse) : juce::Colours::grey.withAlpha(0.3f);

        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = startAngle + sliderPos * (endAngle - startAngle);
        auto lineW = 1.5f + (audioLevel * 2.0f);
        auto arcRadius = radius - 2.0f;

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

        g.setColour(slider.isEnabled() ? baseColor.withAlpha(0.7f + pulse) : juce::Colours::grey);
        g.setFont(juce::Font("Impact", 13.0f, juce::Font::plain));
        g.drawText(slider.getName(), x, y + height - 15, width, 15, juce::Justification::centred);
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        auto bounds = button.getLocalBounds().toFloat();
        auto tickArea = bounds.removeFromLeft(bounds.getHeight()).reduced(4);

        float pulse = audioLevel * 0.5f;
        auto baseColor = juce::Colours::white.interpolatedWith(juce::Colours::blue, juce::jlimit(0.0f, 1.0f, audioLevel * 1.5f));
        
        auto fill = button.getToggleState() ? baseColor.withAlpha(0.7f + pulse) : baseColor.withAlpha(0.1f + pulse * 0.2f);
        
        g.setColour(fill);
        g.drawRect(tickArea, 1.0f + pulse);
        if (button.getToggleState()) g.fillRect(tickArea.reduced(2.0f - pulse));

        if (button.getButtonText().isNotEmpty())
        {
            g.setColour(baseColor.withAlpha(0.8f + pulse * 0.2f));
            g.setFont(juce::Font("Impact", 12.0f, juce::Font::plain));
            g.drawText(button.getButtonText(), bounds.translated(5, 0), juce::Justification::centredLeft);
        }
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonWidth, int buttonHeight, juce::ComboBox& box) override
    {
        juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonWidth, buttonHeight);
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

        g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
        g.fillRect(bounds);

        g.setColour(box.findColour(juce::ComboBox::outlineColourId));
        g.drawRect(bounds, 1.0f);

        juce::Path path;
        float arrowSize = 3.0f;
        float centerX = (float)width - 10.0f;
        float centerY = (float)height * 0.5f;
        path.addTriangle(centerX - arrowSize, centerY - arrowSize * 0.5f,
                         centerX + arrowSize, centerY - arrowSize * 0.5f,
                         centerX, centerY + arrowSize * 0.5f);

        g.setColour(box.findColour(juce::ComboBox::arrowColourId));
        g.fillPath(path);
    }

    juce::Font getComboBoxFont(juce::ComboBox& box) override
    {
        juce::ignoreUnused(box);
        return juce::Font("Impact", 13.0f, juce::Font::plain);
    }

    juce::Font getPopupMenuFont() override
    {
        return juce::Font("Impact", 13.0f, juce::Font::plain);
    }

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        g.fillAll(juce::Colour(0xFF0A0A0A));
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRect(0, 0, width, height, 1);
    }

    juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(juce::ComboBox& box, juce::Label& label) override
    {
        return juce::PopupMenu::Options()
            .withTargetComponent(&box)
            .withMinimumWidth(box.getWidth())
            .withMaximumNumColumns(1)
            .withStandardItemHeight(label.getHeight())
            .withPreferredPopupDirection(juce::PopupMenu::Options::PopupDirection::downwards);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(backgroundColour);
        auto bounds = button.getLocalBounds().toFloat();
        
        // Draw an outline so it's always visible
        g.setColour(juce::Colours::white.withAlpha(0.2f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 2.0f, 1.0f);

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        {
            g.setColour(juce::Colours::white.withAlpha(shouldDrawButtonAsDown ? 0.3f : 0.15f));
            g.fillRoundedRectangle(bounds, 2.0f);
        }
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
        g.setColour(juce::Colours::white.withAlpha(button.isEnabled() ? 1.0f : 0.5f));
        g.setFont(juce::Font("Impact", button.getHeight() * 0.8f, juce::Font::plain));
        g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred);
    }

private:
    float audioLevel = 0.0f;
};
