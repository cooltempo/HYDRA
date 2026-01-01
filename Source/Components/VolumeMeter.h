/*
  ==============================================================================

    VolumeMeter.h
    Created: 18 Oct 2023 4:43:20pm

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../../atomicengine/bsfx/VolumeMeter.h"

//==============================================================================
class VolumeMeter
: public juce::Component
, private juce::Timer
{
public:
    VolumeMeter();
    ~VolumeMeter() override;

    void setProcessorAndChannel(bsfx::VolumeMeter* processor, int channel);

    void timerCallback() override;
    void paint (juce::Graphics&) override;
   
private:
    void updateMeterValue(float value);
    bool isMeterOK() { return processor && !processor->areParamsBad(); }
    
    bsfx::VolumeMeter* processor = nullptr;
    int channel;
    float lastValue = -1.0f;
    
    float value = 0.0f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VolumeMeter)
};

//==============================================================================
class HydraVolumeMeter
: public juce::Component
, private juce::Timer
{
public:
    HydraVolumeMeter();
    ~HydraVolumeMeter();
    
    void setTitle(juce::String string);
    void setProcessor(bsfx::VolumeMeter* p);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;
    
private:
    juce::Label title, level, db;
    VolumeMeter leftMeter, rightMeter;
    bsfx::VolumeMeter* processor = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HydraVolumeMeter)
};
