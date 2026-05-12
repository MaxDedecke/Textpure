/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ParticleSwarm.h"
#include "TextpureLookAndFeel.h"

//==============================================================================
//==============================================================================
/**
    A custom spinner component for selecting frequency bands (Low, Mid, High, Full).
*/
class BandSpinner : public juce::Component
{
public:
    BandSpinner(juce::AudioProcessorValueTreeState& vts, juce::String paramID)
        : param(vts.getParameter(paramID)),
          attachment(*param, [this](float val) { updateLabel(val); })
    {
        jassert(param != nullptr);
        
        valueStrings = param->getAllValueStrings();

        auto setupButton = [this](juce::TextButton& b, juce::String text) {
            b.setButtonText(text);
            b.setConnectedEdges(juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight | juce::Button::ConnectedOnTop | juce::Button::ConnectedOnBottom);
            addAndMakeVisible(b);
        };

        setupButton(leftButton, "<");
        setupButton(rightButton, ">");

        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font("Impact", 12.0f, juce::Font::plain));
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(label);

        leftButton.onClick = [this]() { cycle(-1); };
        rightButton.onClick = [this]() { cycle(1); };

        updateLabel(param->getValue() * (float)(valueStrings.size() - 1));
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        leftButton.setBounds(bounds.removeFromLeft(12).withSizeKeepingCentre(12, 12));
        rightButton.setBounds(bounds.removeFromRight(12).withSizeKeepingCentre(12, 12));
        label.setBounds(bounds);
    }

    void setButtonLookAndFeel(juce::LookAndFeel* lf)
    {
        leftButton.setLookAndFeel(lf);
        rightButton.setLookAndFeel(lf);
    }

private:
    void cycle(int delta)
    {
        int numItems = valueStrings.size();
        if (numItems <= 1) return;
        
        int currentIndex = juce::roundToInt(param->getValue() * (float)(numItems - 1));
        int newIndex = (currentIndex + delta + numItems) % numItems;
        attachment.setValueAsCompleteGesture((float)newIndex);
    }

    void updateLabel(float value)
    {
        int numItems = valueStrings.size();
        if (numItems == 0) return;
        int index = juce::jlimit(0, numItems - 1, juce::roundToInt(value));
        label.setText(valueStrings[index].toUpperCase(), juce::dontSendNotification);
    }

    juce::RangedAudioParameter* param;
    juce::ParameterAttachment attachment;
    juce::StringArray valueStrings;
    juce::TextButton leftButton, rightButton;
    juce::Label label;
};

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

    ParticleSwarm swarm;

    juce::Slider sizeSlider, densitySlider, pitchSlider, textureSlider, mixSlider, reverbSlider;
    juce::ToggleButton sizeBypassButton, densityBypassButton, pitchBypassButton, textureBypassButton, reverbBypassButton;
    juce::ToggleButton sizeSyncButton, syncButton;
    juce::ComboBox sizeRateSelector, rateSelector;
    
    std::unique_ptr<BandSpinner> granularBandSelector, textureBandSelector, reverbBandSelector;
    
    juce::ComboBox presetSelector;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    std::unique_ptr<SliderAttachment> sizeAttachment, densityAttachment, pitchAttachment, textureAttachment, mixAttachment, reverbAttachment;
    std::unique_ptr<ButtonAttachment> sizeBypassAttachment, densityBypassAttachment, pitchBypassAttachment, textureBypassAttachment, reverbBypassAttachment;
    std::unique_ptr<ButtonAttachment> sizeSyncAttachment, syncAttachment;
    std::unique_ptr<ChoiceAttachment> sizeRateAttachment, rateAttachment;

    TextpureLookAndFeel customLookAndFeel;

    void updatePresetList();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
