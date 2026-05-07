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

    syncButton.setButtonText(""); addAndMakeVisible(syncButton);
    rateSelector.addItemList(audioProcessor.apvts.getParameter("RATE")->getAllValueStrings(), 1);
    addAndMakeVisible(rateSelector);
    rateSelector.setJustificationType(juce::Justification::centred);

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

    // Visual Bypass Feedback
    sizeSlider.setEnabled(audioProcessor.apvts.getRawParameterValue("SIZE_BYPASS")->load() < 0.5f);
    densitySlider.setEnabled(audioProcessor.apvts.getRawParameterValue("DENSITY_BYPASS")->load() < 0.5f);
    pitchSlider.setEnabled(audioProcessor.apvts.getRawParameterValue("PITCH_BYPASS")->load() < 0.5f);
    textureSlider.setEnabled(audioProcessor.apvts.getRawParameterValue("TEXTURE_BYPASS")->load() < 0.5f);
    reverbSlider.setEnabled(audioProcessor.apvts.getRawParameterValue("REVERB_BYPASS")->load() < 0.5f);

    // Sync/Rate visibility logic
    bool isSync = audioProcessor.apvts.getRawParameterValue("SYNC")->load() > 0.5f;
    rateSelector.setVisible(isSync);
    densitySlider.setVisible(!isSync);

    repaint();
}

void NewProjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF0A0A0A)); // Deep Black

    auto bounds = getLocalBounds().toFloat();
    
    // Header
    auto headerArea = bounds.removeFromTop(60).reduced(20, 0);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Impact", 32.0f, juce::Font::plain));
    g.drawText("TEXTPURE", headerArea, juce::Justification::centredLeft);
    
    g.setFont(juce::Font("Impact", 12.0f, juce::Font::plain));
    g.drawText("PROD BY JMD", headerArea, juce::Justification::centredRight);
    
    g.setColour(juce::Colours::white.withAlpha(0.1f));
    g.drawHorizontalLine(60, 0, bounds.getWidth());
}

void NewProjectAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    auto headerArea = bounds.removeFromTop(60);
    
    auto mainArea = bounds;
    auto leftControls = mainArea.removeFromLeft(120);
    auto rightControls = mainArea.removeFromRight(120);
    auto bottomArea = mainArea.removeFromBottom(100);
    
    // Central Swarm
    swarm.setBounds(mainArea.reduced(10));
    
    // Side Controls
    int h = leftControls.getHeight() / 3;
    sizeSlider.setBounds(leftControls.removeFromTop(h).reduced(10));
    densitySlider.setBounds(leftControls.removeFromTop(h).reduced(10));
    pitchSlider.setBounds(leftControls.removeFromTop(h).reduced(10));
    
    h = rightControls.getHeight() / 3;
    textureSlider.setBounds(rightControls.removeFromTop(h).reduced(10));
    reverbSlider.setBounds(rightControls.removeFromTop(h).reduced(10));
    mixSlider.setBounds(rightControls.removeFromTop(h).reduced(10));
    
    // Bottom Area
    presetSelector.setBounds(bottomArea.withSizeKeepingCentre(300, 40));
    
    // Position Sync controls over/near density slider
    auto densityBounds = densitySlider.getBounds();
    syncButton.setBounds(densityBounds.removeFromTop(15).removeFromRight(15));
    rateSelector.setBounds(densityBounds.withSizeKeepingCentre(80, 20));

    // Tiny bypass buttons near sliders
    auto placeBypass = [](juce::Slider& s, juce::ToggleButton& b) {
        b.setBounds(s.getBounds().removeFromTop(15).removeFromRight(15));
    };
    
    placeBypass(sizeSlider, sizeBypassButton);
    placeBypass(densitySlider, densityBypassButton);
    placeBypass(pitchSlider, pitchBypassButton);
    placeBypass(textureSlider, textureBypassButton);
    placeBypass(reverbSlider, reverbBypassButton);
    
    // Minimalist Preset Selector
    presetSelector.setBounds(bottomArea.withSizeKeepingCentre(180, 25));
}
