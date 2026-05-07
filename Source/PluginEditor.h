/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class NewProjectAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    NewProjectAudioProcessorEditor (NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    NewProjectAudioProcessor& audioProcessor;

    juce::Slider sizeSlider, densitySlider, pitchSlider, textureSlider, mixSlider, reverbSlider;
    juce::ToggleButton sizeBypassButton, densityBypassButton, pitchBypassButton, textureBypassButton, reverbBypassButton;
    juce::ComboBox presetSelector;
    juce::Label sizeLabel, densityLabel, pitchLabel, textureLabel, mixLabel, reverbLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    std::unique_ptr<SliderAttachment> sizeAttachment, densityAttachment, pitchAttachment, textureAttachment, mixAttachment, reverbAttachment;
    std::unique_ptr<ButtonAttachment> sizeBypassAttachment, densityBypassAttachment, pitchBypassAttachment, textureBypassAttachment, reverbBypassAttachment;
    std::unique_ptr<ChoiceAttachment> presetAttachment;

    float currentGlow{ 0.0f };

    class CustomLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        float glowAmount{ 0.0f };
        CustomLookAndFeel() {
            setColour(juce::Slider::thumbColourId, juce::Colour(0xff00fff2));
            setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff008b82));
            setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333333));
            setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff00fff2));
            setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff222222));
            setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff00fff2));
        }

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                              float rotaryStartAngle, float rotaryEndAngle, juce::Slider& s) override
        {
            auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
            auto centreX = (float)x + (float)width * 0.5f;
            auto centreY = (float)y + (float)height * 0.5f;
            auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

            g.setColour(s.findColour(juce::Slider::rotarySliderOutlineColourId));
            g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

            auto baseColor = s.findColour(juce::Slider::rotarySliderFillColourId);
            auto glowColor = juce::Colour(0xff00fff2).interpolatedWith(juce::Colours::white, 0.2f);
            g.setColour(baseColor.interpolatedWith(glowColor, glowAmount));
            
            juce::Path p; p.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
            g.strokePath(p, juce::PathStrokeType(4.0f + glowAmount * 2.0f));

            g.setColour(s.findColour(juce::Slider::thumbColourId).interpolatedWith(juce::Colours::white, glowAmount));
            juce::Path thumb; thumb.addRectangle(-2.0f, -radius, 4.0f, radius * 0.5f);
            g.fillPath(thumb, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        }
    };

    CustomLookAndFeel customLookAndFeel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
