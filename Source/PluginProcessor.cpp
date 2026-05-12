#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
     : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
       apvts(*this, nullptr, "PARAMETERS", createParameters()),
       granularEngine(128)
{
}

NewProjectAudioProcessor::~NewProjectAudioProcessor() 
{
}

juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SIZE", "Grain Size", 10.0f, 500.0f, 100.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DENSITY", "Density", 1.0f, 50.0f, 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("PITCH", "Pitch", 0.5f, 2.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("TEXTURE", "Texture", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("REVERB", "Reverb", 0.0f, 1.0f, 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterBool>("SIZE_BYPASS", "Size", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>("DENSITY_BYPASS", "Density", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>("PITCH_BYPASS", "Pitch", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>("TEXTURE_BYPASS", "Texture", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>("REVERB_BYPASS", "Reverb", true));

    params.push_back(std::make_unique<juce::AudioParameterBool>("SYNC", "Tempo Sync", false));
    params.push_back(std::make_unique<juce::AudioParameterBool>("SIZE_SYNC", "Size Sync", false));
    
    juce::StringArray rateList = { 
        "1/4", "1/4T", "1/4D", 
        "1/8", "1/8T", "1/8D", 
        "1/16", "1/16T", "1/16D", 
        "1/32", "1/64", "1/128" 
    };
    params.push_back(std::make_unique<juce::AudioParameterChoice>("RATE", "Sync Rate", rateList, 6)); // Default 1/16
    params.push_back(std::make_unique<juce::AudioParameterChoice>("SIZE_RATE", "Size Sync Rate", rateList, 3)); // Default 1/8

    juce::StringArray bandList = { "Full", "Low", "Mid", "High" };
    params.push_back(std::make_unique<juce::AudioParameterChoice>("GRANULAR_BAND", "Granular Band", bandList, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("REVERB_BAND", "Reverb Band", bandList, 0));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("TEXTURE_BAND", "Texture Band", bandList, 0));

    return { params.begin(), params.end() };
}

void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    granularEngine.prepare(sampleRate, samplesPerBlock);
    reverb.setSampleRate(sampleRate);
    reverbFilter.prepare(sampleRate, samplesPerBlock);
    
    setLatencySamples(granularEngine.getLatencySamples());
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
    const bool reverbActive = apvts.getRawParameterValue("REVERB_BYPASS")->load() > 0.5f;
    const bool textureActive = apvts.getRawParameterValue("TEXTURE_BYPASS")->load() > 0.5f;

    const float effectivePitch = (apvts.getRawParameterValue("PITCH_BYPASS")->load() > 0.5f) ? apvts.getRawParameterValue("PITCH")->load() : 1.0f;
    const float effectiveSize = (apvts.getRawParameterValue("SIZE_BYPASS")->load() > 0.5f) ? apvts.getRawParameterValue("SIZE")->load() : 100.0f;
    const float effectiveDensity = (apvts.getRawParameterValue("DENSITY_BYPASS")->load() > 0.5f) ? apvts.getRawParameterValue("DENSITY")->load() : 20.0f;

    const bool syncEnabled = apvts.getRawParameterValue("SYNC")->load() > 0.5f;
    const int rateIndex = (int)apvts.getRawParameterValue("RATE")->load();
    
    const bool sizeSyncEnabled = apvts.getRawParameterValue("SIZE_SYNC")->load() > 0.5f;
    const int sizeRateIndex = (int)apvts.getRawParameterValue("SIZE_RATE")->load();
    
    const int granularBand = (int)apvts.getRawParameterValue("GRANULAR_BAND")->load();
    const int textureBand = (int)apvts.getRawParameterValue("TEXTURE_BAND")->load();
    const int reverbBand = (int)apvts.getRawParameterValue("REVERB_BAND")->load();

    // Get BPM from Host
    double bpm = 120.0;
    if (auto* ph = getPlayHead()) {
        if (auto pos = ph->getPosition()) {
            if (auto b = pos->getBpm()) bpm = *b;
        }
    }

    // --- 2. Keep Dry Signal ---
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // --- 3. Process Granular ---
    granularEngine.process(buffer, effectiveSize, effectiveDensity, effectivePitch, 
                           textureValue, !textureActive, 
                           syncEnabled, rateIndex, 
                           sizeSyncEnabled, sizeRateIndex,
                           granularBand, textureBand, 
                           bpm);

    // --- 4. Mix Dry/Wet ---
    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.applyGain(ch, 0, numSamples, mix);
        buffer.addFrom(ch, 0, dryBuffer, ch, 0, numSamples, 1.0f - mix);
    }

    // --- 5. Reverb ---
    if (reverbActive && reverbValue > 0.01f && numChannels >= 2)
    {
        if (reverbBand == 0) // Full
        {
            updateReverbParameters(reverbValue);
            reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), numSamples);
        }
        else // Band Restricted
        {
            juce::AudioBuffer<float> wetBuffer;
            wetBuffer.makeCopyOf(buffer);
            
            reverbParams.dryLevel = 0.0f; 
            reverbParams.wetLevel = reverbValue;
            reverbParams.roomSize = 0.5f + (reverbValue * 0.4f);
            reverb.setParameters(reverbParams);
            
            reverb.processStereo(wetBuffer.getWritePointer(0), wetBuffer.getWritePointer(1), numSamples);
            reverbFilter.process(wetBuffer, reverbBand);
            
            for (int ch = 0; ch < numChannels; ++ch)
                buffer.addFrom(ch, 0, wetBuffer, ch, 0, numSamples);
        }
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
int NewProjectAudioProcessor::getNumPrograms() { return 1; }
int NewProjectAudioProcessor::getCurrentProgram() { return 0; }
const juce::String NewProjectAudioProcessor::getProgramName (int index) { return "Default"; }
void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName) {}
void NewProjectAudioProcessor::setCurrentProgram (int index) {}

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
