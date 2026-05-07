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
    juce::ToggleButton syncButton;
    juce::ComboBox rateSelector;
    juce::ComboBox presetSelector;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ChoiceAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    std::unique_ptr<SliderAttachment> sizeAttachment, densityAttachment, pitchAttachment, textureAttachment, mixAttachment, reverbAttachment;
    std::unique_ptr<ButtonAttachment> sizeBypassAttachment, densityBypassAttachment, pitchBypassAttachment, textureBypassAttachment, reverbBypassAttachment;
    std::unique_ptr<ButtonAttachment> syncAttachment;
    std::unique_ptr<ChoiceAttachment> rateAttachment;
    std::unique_ptr<ChoiceAttachment> presetAttachment;

    TextpureLookAndFeel customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NewProjectAudioProcessorEditor)
};
