/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../atomicengine/AtomicEngine.h"
#include "../atomicengine/bsfx/Graph.h"
#include "../atomicengine/bsfx/GainDB.h"
#include "../atomicengine/bsfx/VolumeMeter.h"
#include "Visualiser/VisualiserProcessor.h"

//==============================================================================
class HydraAudioProcessor
: public juce::AudioProcessor
, public juce::AudioProcessorParameter::Listener
{
public:
    //==============================================================================
    HydraAudioProcessor();
    ~HydraAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
  
    //==============================================================================
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;
    
    juce::AudioParameterBool* paramMacroMap;
    juce::AudioParameterBool* paramBypass;
    juce::AudioParameterBool* paramAtomic;
    juce::AudioParameterBool* paramFreeze;
    
    juce::AudioParameterFloat* paramInputGain;
    juce::AudioParameterFloat* paramMix;
    juce::AudioParameterFloat* paramOutputGain;

    bsfx::Graph graph;
    AtomicEngine* engine;
    bsfx::VolumeMeter* inputMeter;
    bsfx::VolumeMeter* outputMeter;
    bsfx::GainDB* inputGain;
    bsfx::GainDB* outputGain;
    
    VisualiserProcessor visualiserProcessor;
    
    juce::Rectangle<int> editorWindow{980, 765};

private:
    juce::CriticalSection criticalSection;
    
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    std::unique_ptr<juce::AudioProcessorValueTreeState> parameters;
    void createParameters();
    void addParameters();
    
    void updatePluginParams();
    
    template<typename T>
    void addParam(T* param)
    {
        layout.add(std::unique_ptr<T>(param));
    }
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HydraAudioProcessor)
};
