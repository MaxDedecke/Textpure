/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&customLookAndFeel);

    auto setupSlider = [this](juce::Slider& s, juce::Label& l, juce::String name) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        addAndMakeVisible(s);
        l.setText(name, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.attachToComponent(&s, false);
        addAndMakeVisible(l);
    };

    setupSlider(sizeSlider, sizeLabel, "Grain Size");
    setupSlider(densitySlider, densityLabel, "Density");
    setupSlider(pitchSlider, pitchLabel, "Pitch");
    setupSlider(textureSlider, textureLabel, "Texture");
    setupSlider(mixSlider, mixLabel, "Mix");
    setupSlider(reverbSlider, reverbLabel, "Reverb");

    sizeBypassButton.setButtonText("Bypass"); addAndMakeVisible(sizeBypassButton);
    densityBypassButton.setButtonText("Bypass"); addAndMakeVisible(densityBypassButton);
    pitchBypassButton.setButtonText("Bypass"); addAndMakeVisible(pitchBypassButton);
    textureBypassButton.setButtonText("Bypass"); addAndMakeVisible(textureBypassButton);
    reverbBypassButton.setButtonText("Bypass"); addAndMakeVisible(reverbBypassButton);

    presetSelector.addItemList(audioProcessor.apvts.getParameter("PRESET")->getAllValueStrings(), 1);
    addAndMakeVisible(presetSelector);
    presetSelector.onChange = [this]() { audioProcessor.setCurrentProgram(presetSelector.getSelectedItemIndex()); };

    sizeAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "SIZE", sizeSlider);
    densityAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DENSITY", densitySlider);
    pitchAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "PITCH", pitchSlider);
    textureAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "TEXTURE", textureSlider);
    mixAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "MIX", mixSlider);
    reverbAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "REVERB", reverbSlider);

    sizeBypassAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "SIZE_BYPASS", sizeBypassButton);
    densityBypassAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "DENSITY_BYPASS", densityBypassButton);
    pitchBypassAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "PITCH_BYPASS", pitchBypassButton);
    textureBypassAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "TEXTURE_BYPASS", textureBypassButton);
    reverbBypassAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "REVERB_BYPASS", reverbBypassButton);
    
    presetAttachment = std::make_unique<ChoiceAttachment>(audioProcessor.apvts, "PRESET", presetSelector);

    startTimerHz(30); 
    setSize (650, 550);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor() { setLookAndFeel(nullptr); }

void NewProjectAudioProcessorEditor::timerCallback()
{
    float level = audioProcessor.currentLevel.load();
    float targetGlow = juce::jmin(1.0f, level * 15.0f); // Noch höhere Sensitivität
    currentGlow = currentGlow + (targetGlow - currentGlow) * 0.15f;
    customLookAndFeel.glowAmount = currentGlow;
    repaint();
}

void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient gradient(juce::Colour(0xff050505), 0, 0, juce::Colour(0xff151515), 0, (float)getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();


    auto titleArea = getLocalBounds().removeFromTop(80);
    g.setColour (juce::Colour(0xff00fff2).interpolatedWith(juce::Colours::white, currentGlow));
    g.setFont (juce::Font("Impact", 40.0f, juce::Font::plain));
    g.drawFittedText ("TEXTPURE", titleArea.removeFromTop(50), juce::Justification::centred, 1);
    
    g.setColour (juce::Colours::grey.interpolatedWith(juce::Colour(0xff00fff2), currentGlow));
    g.setFont (juce::Font("Arial", 14.0f, juce::Font::italic));
    g.drawFittedText ("prodbyJMD", titleArea, juce::Justification::centred, 1);
    
    g.setColour(juce::Colour(0xff00fff2).withAlpha(0.1f + currentGlow * 0.6f));
    g.drawHorizontalLine(78, 40.0f, (float)getWidth() - 40.0f);
}

void NewProjectAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    bounds.removeFromTop(85);
    presetSelector.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(200, 30));
    bounds.removeFromTop(20);

    auto topRow = bounds.removeFromTop(bounds.getHeight() / 2);
    int sliderWidth = topRow.getWidth() / 3;

    auto setupArea = [](juce::Rectangle<int> area, juce::Slider& s, juce::ToggleButton& b) {
        s.setBounds(area.removeFromTop(area.getHeight() - 25));
        b.setBounds(area.withSize(area.getWidth(), 20).translated(0, -10));
    };

    setupArea(topRow.removeFromLeft(sliderWidth).reduced(15), sizeSlider, sizeBypassButton);
    setupArea(topRow.removeFromLeft(sliderWidth).reduced(15), densitySlider, densityBypassButton);
    setupArea(topRow.reduced(15), pitchSlider, pitchBypassButton);

    auto bottomRow = bounds;
    int bottomSliderWidth = bottomRow.getWidth() / 3;

    setupArea(bottomRow.removeFromLeft(bottomSliderWidth).reduced(15), textureSlider, textureBypassButton);
    mixSlider.setBounds(bottomRow.removeFromLeft(bottomSliderWidth).reduced(15));
    setupArea(bottomRow.reduced(15), reverbSlider, reverbBypassButton);
}
