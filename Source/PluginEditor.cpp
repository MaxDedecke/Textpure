#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor (NewProjectAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&customLookAndFeel);

    addAndMakeVisible(swarm);

    auto setupSlider = [this](juce::Slider& s, juce::String name) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setName(name);
        addAndMakeVisible(s);
    };

    setupSlider(sizeSlider, "SIZE");
    setupSlider(densitySlider, "DENSITY");
    setupSlider(pitchSlider, "PITCH");
    setupSlider(textureSlider, "TEXTURE");
    setupSlider(mixSlider, "MIX");
    setupSlider(reverbSlider, "REVERB");

    sizeBypassButton.setButtonText(""); addAndMakeVisible(sizeBypassButton);
    densityBypassButton.setButtonText(""); addAndMakeVisible(densityBypassButton);
    pitchBypassButton.setButtonText(""); addAndMakeVisible(pitchBypassButton);
    textureBypassButton.setButtonText(""); addAndMakeVisible(textureBypassButton);
    reverbBypassButton.setButtonText(""); addAndMakeVisible(reverbBypassButton);

    sizeSyncButton.setButtonText(""); addAndMakeVisible(sizeSyncButton);
    sizeRateSelector.addItemList(audioProcessor.apvts.getParameter("SIZE_RATE")->getAllValueStrings(), 1);
    addAndMakeVisible(sizeRateSelector);
    sizeRateSelector.setJustificationType(juce::Justification::centred);

    granularBandSelector = std::make_unique<BandSpinner>(audioProcessor.apvts, "GRANULAR_BAND");
    textureBandSelector = std::make_unique<BandSpinner>(audioProcessor.apvts, "TEXTURE_BAND");
    reverbBandSelector = std::make_unique<BandSpinner>(audioProcessor.apvts, "REVERB_BAND");
    
    addAndMakeVisible(*granularBandSelector);
    addAndMakeVisible(*textureBandSelector);
    addAndMakeVisible(*reverbBandSelector);

    presetSelector.addItemList(audioProcessor.apvts.getParameter("PRESET")->getAllValueStrings(), 1);
    addAndMakeVisible(presetSelector);
    presetSelector.setJustificationType(juce::Justification::centred);

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
    
    syncAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "SYNC", syncButton);
    rateAttachment = std::make_unique<ChoiceAttachment>(audioProcessor.apvts, "RATE", rateSelector);
    
    sizeSyncAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "SIZE_SYNC", sizeSyncButton);
    sizeRateAttachment = std::make_unique<ChoiceAttachment>(audioProcessor.apvts, "SIZE_RATE", sizeRateSelector);
    
    presetAttachment = std::make_unique<ChoiceAttachment>(audioProcessor.apvts, "PRESET", presetSelector);

    startTimerHz(60); 
    setSize (700, 500);
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor() { setLookAndFeel(nullptr); }

void NewProjectAudioProcessorEditor::timerCallback()
{
    float level = audioProcessor.getCurrentLevel();
    
    customLookAndFeel.setAudioLevel(level);

    swarm.setParameters(
        audioProcessor.apvts.getRawParameterValue("DENSITY")->load(),
        audioProcessor.apvts.getRawParameterValue("TEXTURE")->load(),
        audioProcessor.apvts.getRawParameterValue("SIZE")->load(),
        audioProcessor.apvts.getRawParameterValue("PITCH")->load(),
        level
    );

    // Visual Bypass Feedback (Active logic: parameter=true means enabled)
    sizeSlider.setEnabled(audioProcessor.apvts.getRawParameterValue("SIZE_BYPASS")->load() > 0.5f);
    densitySlider.setEnabled(audioProcessor.apvts.getRawParameterValue("DENSITY_BYPASS")->load() > 0.5f);
    pitchSlider.setEnabled(audioProcessor.apvts.getRawParameterValue("PITCH_BYPASS")->load() > 0.5f);
    textureSlider.setEnabled(audioProcessor.apvts.getRawParameterValue("TEXTURE_BYPASS")->load() > 0.5f);
    reverbSlider.setEnabled(audioProcessor.apvts.getRawParameterValue("REVERB_BYPASS")->load() > 0.5f);

    // Sync/Rate visibility logic
    bool isSync = audioProcessor.apvts.getRawParameterValue("SYNC")->load() > 0.5f;
    rateSelector.setVisible(isSync);
    densitySlider.setVisible(!isSync);

    bool isSizeSync = audioProcessor.apvts.getRawParameterValue("SIZE_SYNC")->load() > 0.5f;
    sizeRateSelector.setVisible(isSizeSync);
    sizeSlider.setVisible(!isSizeSync);

    repaint();
}

void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0A0A0A)); // Deep Black

    auto bounds = getLocalBounds().toFloat();
    
    // Header
    auto headerArea = bounds.removeFromTop(80).reduced(20, 0);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Impact", 32.0f, juce::Font::plain));
    g.drawText("TEXTPURE", headerArea, juce::Justification::centredLeft);
    
    g.setFont(juce::Font("Impact", 12.0f, juce::Font::plain));
    g.drawText("PROD BY JMD", headerArea, juce::Justification::centredRight);
    
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawHorizontalLine(80, 0, bounds.getWidth());
}

void NewProjectAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto headerArea = bounds.removeFromTop(80).reduced(20, 0);
    
    // Preset Selector in Header
    presetSelector.setBounds(headerArea.withSizeKeepingCentre(180, 25));

    auto mainArea = bounds.reduced(20, 10);
    auto leftControls = mainArea.removeFromLeft(140);
    auto rightControls = mainArea.removeFromRight(140);
    
    // Central Swarm
    swarm.setBounds(mainArea.reduced(10));
    
    // Side Controls
    int h = leftControls.getHeight() / 3;
    
    // --- Size Area ---
    auto sizeArea = leftControls.removeFromTop(h).reduced(10);
    sizeSlider.setBounds(sizeArea);
    sizeRateSelector.setBounds(sizeArea.withSizeKeepingCentre(80, 20));
    auto sizeTopRow = sizeArea.removeFromTop(15);
    sizeSyncButton.setBounds(sizeTopRow.removeFromLeft(15));
    sizeBypassButton.setBounds(sizeTopRow.removeFromRight(15));
    
    // --- Density Area ---
    auto densityArea = leftControls.removeFromTop(h).reduced(10);
    densitySlider.setBounds(densityArea);
    rateSelector.setBounds(densityArea.withSizeKeepingCentre(80, 20));
    auto densityTopRow = densityArea.removeFromTop(15);
    syncButton.setBounds(densityTopRow.removeFromLeft(15));
    densityBypassButton.setBounds(densityTopRow.removeFromRight(15));
    
    // --- Pitch Area ---
    auto pitchArea = leftControls.removeFromTop(h).reduced(10);
    pitchSlider.setBounds(pitchArea);
    granularBandSelector->setBounds(pitchArea.withSizeKeepingCentre(60, 20));
    pitchBypassButton.setBounds(pitchArea.removeFromTop(15).removeFromRight(15));
    
    h = rightControls.getHeight() / 3;
    
    // --- Texture Area ---
    auto textureArea = rightControls.removeFromTop(h).reduced(10);
    textureSlider.setBounds(textureArea);
    textureBandSelector->setBounds(textureArea.withSizeKeepingCentre(60, 20));
    textureBypassButton.setBounds(textureArea.removeFromTop(15).removeFromRight(15));

    // --- Reverb Area ---
    auto reverbArea = rightControls.removeFromTop(h).reduced(10);
    reverbSlider.setBounds(reverbArea);
    reverbBandSelector->setBounds(reverbArea.withSizeKeepingCentre(60, 20));
    reverbBypassButton.setBounds(reverbArea.removeFromTop(15).removeFromRight(15));

    // --- Mix Area ---
    auto mixArea = rightControls.removeFromTop(h).reduced(10);
    mixSlider.setBounds(mixArea);
}
