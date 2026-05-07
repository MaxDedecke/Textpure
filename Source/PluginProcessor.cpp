#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
     : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
       apvts(*this, nullptr, "PARAMETERS", createParameters()),
       granularEngine(40)
{
}

NewProjectAudioProcessor::~NewProjectAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SIZE", "Grain Size", 10.0f, 500.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DENSITY", "Density", 1.0f, 50.0f, 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("PITCH", "Pitch", 0.5f, 2.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("TEXTURE", "Texture", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERB", "Reverb", 0.0f, 1.0f, 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterBool>("SIZE_BYPASS", "Size Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("DENSITY_BYPASS", "Density Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("PITCH_BYPASS", "Pitch Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("TEXTURE_BYPASS", "Texture Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("REVERB_BYPASS", "Reverb Bypass", false));

    juce::StringArray presetList = { "Default", "Dark Clouds", "Digital Grit", "Ghost Melodies", "Subtle Texture", "Trap Shimmer" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>("PRESET", "Preset", presetList, 0));

    return { params.begin(), params.end() };
}

void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    granularEngine.prepare(sampleRate, samplesPerBlock);
    reverb.setSampleRate(sampleRate);
}

void NewProjectAudioProcessor::releaseResources() {}

bool NewProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
    auto mainIn = layouts.getMainInputChannelSet();
    auto mainOut = layouts.getMainOutputChannelSet();
    
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo()) return false;
    if (mainIn != mainOut) return false;
    
    return true;
}

void NewProjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    if (getSampleRate() <= 0) return;

    // --- 1. Get Parameters ---
    const float mix = apvts.getRawParameterValue("MIX")->load();
    const float reverbValue = apvts.getRawParameterValue("REVERB")->load();
    const float textureValue = apvts.getRawParameterValue("TEXTURE")->load();
    const bool reverbBypass = apvts.getRawParameterValue("REVERB_BYPASS")->load() > 0.5f;
    const bool textureBypass = apvts.getRawParameterValue("TEXTURE_BYPASS")->load() > 0.5f;

    const float effectivePitch = (apvts.getRawParameterValue("PITCH_BYPASS")->load() > 0.5f) ? 1.0f : apvts.getRawParameterValue("PITCH")->load();
    const float effectiveSize = (apvts.getRawParameterValue("SIZE_BYPASS")->load() > 0.5f) ? 100.0f : apvts.getRawParameterValue("SIZE")->load();
    const float effectiveDensity = (apvts.getRawParameterValue("DENSITY_BYPASS")->load() > 0.5f) ? 20.0f : apvts.getRawParameterValue("DENSITY")->load();

    // --- 2. Keep Dry Signal ---
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // --- 3. Process Granular ---
    granularEngine.process(buffer, effectiveSize, effectiveDensity, effectivePitch, textureValue, textureBypass);

    // --- 4. Mix Dry/Wet ---
    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.applyGain(ch, 0, numSamples, mix);
        buffer.addFrom(ch, 0, dryBuffer, ch, 0, numSamples, 1.0f - mix);
    }

    // --- 5. Reverb ---
    if (!reverbBypass && reverbValue > 0.01f && numChannels >= 2)
    {
        updateReverbParameters(reverbValue);
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), numSamples);
    }
}

void NewProjectAudioProcessor::updateReverbParameters(float reverbValue)
{
    reverbParams.dryLevel = 1.0f;
    reverbParams.wetLevel = reverbValue;
    reverbParams.roomSize = 0.5f + (reverbValue * 0.4f);
    reverb.setParameters(reverbParams);
}

const juce::String NewProjectAudioProcessor::getName() const { return JucePlugin_Name; }
int NewProjectAudioProcessor::getNumPrograms() { return 6; }
int NewProjectAudioProcessor::getCurrentProgram() { return (int)apvts.getRawParameterValue("PRESET")->load(); }
const juce::String NewProjectAudioProcessor::getProgramName (int index) {
    juce::StringArray names = { "Default", "Dark Clouds", "Digital Grit", "Ghost Melodies", "Subtle Texture", "Trap Shimmer" };
    return names[index];
}
void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void NewProjectAudioProcessor::setCurrentProgram (int index)
{
    auto setParam = [this](juce::String id, float val) {
        auto* p = apvts.getParameter(id);
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(val));
    };
    auto setBool = [this](juce::String id, bool val) {
        apvts.getParameter(id)->setValueNotifyingHost(val ? 1.0f : 0.0f);
    };

    if (index == 1) { // Dark Clouds
        setParam("SIZE", 450.0f); setParam("DENSITY", 40.0f); setParam("PITCH", 0.5f); setParam("TEXTURE", 0.3f); setParam("MIX", 0.7f); setParam("REVERB", 0.6f);
        setBool("SIZE_BYPASS", false); setBool("DENSITY_BYPASS", false); setBool("PITCH_BYPASS", false); setBool("TEXTURE_BYPASS", false); setBool("REVERB_BYPASS", false);
    } else if (index == 2) { // Digital Grit
        setParam("SIZE", 20.0f); setParam("DENSITY", 50.0f); setParam("PITCH", 1.2f); setParam("TEXTURE", 0.9f); setParam("MIX", 0.4f); setParam("REVERB", 0.1f);
        setBool("SIZE_BYPASS", false); setBool("DENSITY_BYPASS", false); setBool("PITCH_BYPASS", false); setBool("TEXTURE_BYPASS", false); setBool("REVERB_BYPASS", false);
    } else if (index == 3) { // Ghost Melodies
        setParam("SIZE", 250.0f); setParam("DENSITY", 15.0f); setParam("PITCH", 2.0f); setParam("TEXTURE", 0.1f); setParam("MIX", 0.6f); setParam("REVERB", 0.8f);
        setBool("SIZE_BYPASS", false); setBool("DENSITY_BYPASS", false); setBool("PITCH_BYPASS", false); setBool("TEXTURE_BYPASS", false); setBool("REVERB_BYPASS", false);
    } else if (index == 4) { // Subtle Texture
        setParam("SIZE", 80.0f); setParam("DENSITY", 10.0f); setParam("PITCH", 1.0f); setParam("TEXTURE", 0.5f); setParam("MIX", 0.2f); setParam("REVERB", 0.2f);
        setBool("SIZE_BYPASS", false); setBool("DENSITY_BYPASS", false); setBool("PITCH_BYPASS", true); setBool("TEXTURE_BYPASS", false); setBool("REVERB_BYPASS", false);
    } else if (index == 5) { // Trap Shimmer
        setParam("SIZE", 150.0f); setParam("DENSITY", 30.0f); setParam("PITCH", 1.5f); setParam("TEXTURE", 0.2f); setParam("MIX", 0.5f); setParam("REVERB", 0.4f);
        setBool("SIZE_BYPASS", false); setBool("DENSITY_BYPASS", false); setBool("PITCH_BYPASS", false); setBool("TEXTURE_BYPASS", false); setBool("REVERB_BYPASS", false);
    }
}

juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor() { return new NewProjectAudioProcessorEditor (*this); }
bool NewProjectAudioProcessor::hasEditor() const { return true; }

void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {
    auto state = apvts.copyState(); std::unique_ptr<juce::XmlElement> xml (state.createXml()); copyXmlToBinary (*xml, destData);
}
void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType())) apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new NewProjectAudioProcessor(); }
