/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
     : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
       apvts(*this, nullptr, "PARAMETERS", createParameters())
{
    grains.resize(maxGrains);
    DBG("TEXTPURE: Constructor called");
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

const juce::String NewProjectAudioProcessor::getName() const { return JucePlugin_Name; }
int NewProjectAudioProcessor::getNumPrograms() { return 6; }
int NewProjectAudioProcessor::getCurrentProgram() { return (int)apvts.getRawParameterValue("PRESET")->load(); } 
const juce::String NewProjectAudioProcessor::getProgramName (int index) {
    juce::StringArray names = { "Default", "Dark Clouds", "Digital Grit", "Ghost Melodies", "Subtle Texture", "Trap Shimmer" };
    return names[index];
}
void NewProjectAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void NewProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    DBG("TEXTPURE: prepareToPlay - SR: " << sampleRate << " B: " << samplesPerBlock);

    // 2 Sekunden Puffer fuer mehr Stabilitaet
    circularBuffer.setSize(2, (int)(sampleRate * 2.0));
    circularBuffer.clear();
    writePosition = 0; samplesSinceLastGrain = 0;
    for (auto& g : grains) g.reset();
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
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    if (getSampleRate() <= 0) return;

    // Parameter laden
    float mix = apvts.getRawParameterValue("MIX")->load();
    float reverbValue = apvts.getRawParameterValue("REVERB")->load();
    float textureValue = apvts.getRawParameterValue("TEXTURE")->load();
    bool reverbBypass = apvts.getRawParameterValue("REVERB_BYPASS")->load() > 0.5f;
    bool textureBypass = apvts.getRawParameterValue("TEXTURE_BYPASS")->load() > 0.5f;

    // Granular Params
    float effectivePitch = (apvts.getRawParameterValue("PITCH_BYPASS")->load() > 0.5f) ? 1.0f : apvts.getRawParameterValue("PITCH")->load();
    float effectiveSize = (apvts.getRawParameterValue("SIZE_BYPASS")->load() > 0.5f) ? 100.0f : apvts.getRawParameterValue("SIZE")->load();
    float effectiveDensity = (apvts.getRawParameterValue("DENSITY_BYPASS")->load() > 0.5f) ? 20.0f : apvts.getRawParameterValue("DENSITY")->load();

    int circularBufferSize = circularBuffer.getNumSamples();
    int grainDurationSamples = (int)(getSampleRate() * (effectiveSize / 1000.0f));
    float grainIntervalSamples = (float)getSampleRate() / effectiveDensity;

    float totalLevel = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Sicherer Zugriff auf Input
        float inL = (numChannels > 0) ? buffer.getSample(0, sample) : 0.0f;
        float inR = (numChannels > 1) ? buffer.getSample(1, sample) : inL;

        circularBuffer.setSample(0, writePosition, inL);
        circularBuffer.setSample(1, writePosition, inR);
        totalLevel += std::abs(inL);

        // Grain Spawning
        samplesSinceLastGrain++;
        if (samplesSinceLastGrain >= grainIntervalSamples)
        {
            samplesSinceLastGrain = 0;
            for (auto& g : grains) {
                if (!g.isActive) {
                    g.isActive = true;
                    g.durationSamples = juce::jmax(10, grainDurationSamples);
                    g.ageSamples = 0;
                    g.pitch = effectivePitch;

                    float jitter = textureBypass ? 0.0f : (juce::Random::getSystemRandom().nextFloat() - 0.5f) * textureValue * 5000.0f;
                    g.startSample = (float)writePosition - (g.durationSamples * g.pitch) - 500.0f - jitter;     

                    while (g.startSample < 0) g.startSample += (float)circularBufferSize;
                    g.currentSample = g.startSample;
                    break;
                }
            }
        }

        // Grains berechnen
        float grainOutL = 0, grainOutR = 0;
        int activeGrains = 0;
        for (auto& g : grains) {
            if (g.isActive) {
                grainOutL += g.getSampleForChannel(circularBuffer, 0, circularBufferSize);
                grainOutR += g.getSampleForChannel(circularBuffer, 1, circularBufferSize);
                g.updateState(circularBufferSize);
                activeGrains++;
            }
        }

        // Normalisierung
        if (activeGrains > 0) {
            float norm = 1.0f / std::sqrt((float)activeGrains);
            grainOutL *= norm;
            grainOutR *= norm;
        }

        // Texture
        if (!textureBypass && textureValue > 0.05f) {
            float drive = 1.0f + textureValue * 2.0f;
            grainOutL = std::tanh(grainOutL * drive);
            grainOutR = std::tanh(grainOutR * drive);
        }

        // Finaler Output
        if (totalNumOutputChannels > 0) buffer.setSample(0, sample, (inL * (1.0f - mix)) + (grainOutL * mix));  
        if (totalNumOutputChannels > 1) buffer.setSample(1, sample, (inR * (1.0f - mix)) + (grainOutR * mix));  

        writePosition = (writePosition + 1) % circularBufferSize;
    }

    // Reverb (Post-Mix)
    if (!reverbBypass && reverbValue > 0.01f && totalNumOutputChannels >= 2) {
        reverbParams.dryLevel = 1.0f;
        reverbParams.wetLevel = reverbValue;
        reverbParams.roomSize = 0.5f + (reverbValue * 0.4f);
        reverb.setParameters(reverbParams);
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
    }

    // Animation Update
    currentLevel.store(totalLevel / (float)buffer.getNumSamples());
}

bool NewProjectAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor() { return new NewProjectAudioProcessorEditor (*this); }

void NewProjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {
    auto state = apvts.copyState(); std::unique_ptr<juce::XmlElement> xml (state.createXml()); copyXmlToBinary (*xml, destData);
}
void NewProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType())) apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new NewProjectAudioProcessor(); }
