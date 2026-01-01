/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Components/ResizeableEditor.h"

#include "../atomicengine/bsfx/Parameters.h"
#include "../atomicengine/bsfx/SetParameterValue.h"

//==============================================================================
HydraAudioProcessor::HydraAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    engine = new AtomicEngine;
    graph.addNodeToGraph(engine);
    
    inputMeter = new bsfx::VolumeMeter;
    graph.addNodeToGraph(inputMeter);
    
    outputMeter = new bsfx::VolumeMeter;
    graph.addNodeToGraph(outputMeter);
    
    inputGain = new bsfx::GainDB(-12.0f, 12.0f, 0.0f);
    graph.addNodeToGraph(inputGain);
    
    outputGain = new bsfx::GainDB(-12.0f, 12.0f, 0.0f);
    graph.addNodeToGraph(outputGain);
    
    graph.addChainToGraph(graph.getInputNodeID(),
                          inputGain->nodeID,
                          inputMeter->nodeID,
                          engine->nodeID,
                          outputGain->nodeID,
                          outputMeter->nodeID,
                          graph.getOutputNodeID());
 
    juce::XmlDocument doc(BinaryData::DefaultPreset_xml);
    auto rootEl = doc.getDocumentElement();
    engine->loadPreset(rootEl.get());
    
    auto meterPresetXml = juce::XmlDocument::parse(juce::String(BinaryData::meter_xml,
                                                                BinaryData::meter_xmlSize));
    inputMeter->loadPresetXml(engine, meterPresetXml.get());
    outputMeter->loadPresetXml(engine, meterPresetXml.get());
    
    createParameters();
    addParameters();
    updatePluginParams();
    
    auto presetXml = juce::XmlDocument::parse(juce::String(BinaryData::visualiser_xml,
                                                           BinaryData::visualiser_xmlSize));
    visualiserProcessor.loadPresetXml(engine, presetXml.get());
}

HydraAudioProcessor::~HydraAudioProcessor()
{
}

//==============================================================================
void HydraAudioProcessor::createParameters()
{
    auto createBoolParam = [](juce::String name, juce::String id, bool def = false) {
        return new juce::AudioParameterBool(juce::ParameterID(id, 1),
                                            name,
//                                            PresetManager::getDefaultValue(id)
                                            def
                                            );
    };
    
    auto createFloatParam = [](juce::String name, juce::String id) {
        return new juce::AudioParameterFloat(juce::ParameterID(id, 1),
                                             name,
                                             juce::NormalisableRange<float>(0.0f, 1.0f),
                                             0.5f,
//                                            PresetManager::getDefaultValue(id)
                                             juce::AudioParameterFloatAttributes()
                                             .withStringFromValueFunction(bsfx::decimalToString<2>)
                                            );
    };

    paramMacroMap = createBoolParam("Macro Map", "Macro Map");
    paramBypass = createBoolParam("Bypass", "Bypass");
    paramAtomic = createBoolParam("Atomic", "Atomic", true);
    paramFreeze = createBoolParam("Freeze", "Freeze");
    
    paramInputGain = bsfx::newGainParameter(juce::ParameterID("Input Gain", 1), "Input Gain", juce::Decibels::decibelsToGain(-12.0f), juce::Decibels::decibelsToGain(12.0f), 1.0f);
    paramMix = createFloatParam("Mix", "Mix");
    paramOutputGain = bsfx::newGainParameter(juce::ParameterID("Output Gain", 1), "Output Gain", juce::Decibels::decibelsToGain(-12.0f), juce::Decibels::decibelsToGain(12.0f), 1.0f);
}

void HydraAudioProcessor::addParameters()
{
    addParam(paramMacroMap);
    addParam(paramBypass);
    addParam(paramAtomic);
    addParam(paramFreeze);
    addParam(paramInputGain);
    addParam(paramMix);
    addParam(paramOutputGain);
    
    engine->forEachMacroParameter([this] (MacroParameter& mp) {
        addParam(mp.createPluginParameter());
    });
    
    parameters = std::make_unique<juce::AudioProcessorValueTreeState>(*this, nullptr, "Hydra", std::move(layout));
    
    for (auto p : getParameters())
        p->addListener(this);
}

//==============================================================================
const juce::String HydraAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HydraAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HydraAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HydraAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HydraAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HydraAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HydraAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HydraAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HydraAudioProcessor::getProgramName (int index)
{
    return {};
}

void HydraAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void HydraAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ScopedLock lock(criticalSection);
    graph.prepareToPlay(sampleRate, samplesPerBlock);
}

void HydraAudioProcessor::reset()
{
    juce::ScopedLock lock(criticalSection);
    graph.reset();
}

void HydraAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HydraAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Only mono-mono and mono-stereo are supported
    
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    return true;
}
#endif

void HydraAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedLock lock(criticalSection);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Only mono-mono and mono-stereo are supported
    jassert(totalNumInputChannels == 1 || totalNumInputChannels == 2);
    jassert(totalNumOutputChannels == 2);
    jassert(buffer.getNumChannels() == 2);
    
    // If input is mono, copy it to both channels
    if (totalNumInputChannels == 1)
    {
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
    }
    
    graph.processBlock(buffer, midiMessages);
}

//==============================================================================
bool HydraAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HydraAudioProcessor::createEditor()
{
//    return new HydraAudioProcessorEditor (*this);
    return new ResizeableEditor(*this, new HydraAudioProcessorEditor(*this));
}

//==============================================================================
void HydraAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml("Hydra");
    
    auto paramXml = parameters->state.createXml();
    xml.addChildElement(paramXml.release());
    
    auto engineXml = engine->getStateXml();
    xml.addChildElement(engineXml.release());
    
    auto visXML = visualiserProcessor.getPresetXml();
    xml.addChildElement(visXML.release());
    
    DBG(xml.toString());
    
    copyXmlToBinary(xml, destData);
}

void HydraAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml)
    {
        auto paramXml = xml->getChildByName("Hydra");
        if (paramXml)
        {
            auto paramTree = juce::ValueTree::fromXml(*paramXml);
            parameters->replaceState(paramTree);
        }
        
        auto engineXml = xml->getChildByName("AtomicEngine");
        if (engineXml)
        {
            engine->setStateXml(*engineXml);
        }
        
        auto visXML = xml->getChildByName("Processor");
        if (visXML)
        {
            visualiserProcessor.loadPresetXml(engine, visXML);
        }
    }
}

//==============================================================================
void HydraAudioProcessor::parameterValueChanged (int parameterIndex, float newValue)
{
    updatePluginParams();
}

void HydraAudioProcessor::parameterGestureChanged (int parameterIndex, bool gestureIsStarting)
{
    
}
//==============================================================================
void HydraAudioProcessor::updatePluginParams()
{
    engine->setAnalysisFrozen(paramFreeze->get());
    engine->setAtomicMode(paramAtomic->get());
    bsfx::setParameterValue(engine->paramMix, paramMix->get());
    bsfx::setParameterValue(graph.paramBypass, paramBypass->get());
    bsfx::setParameterValue(inputGain->paramGain, juce::Decibels::gainToDecibels(paramInputGain->get()));
    bsfx::setParameterValue(outputGain->paramGain, juce::Decibels::gainToDecibels(paramOutputGain->get()));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HydraAudioProcessor();
}
