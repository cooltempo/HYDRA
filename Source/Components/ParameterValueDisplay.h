/*
  ==============================================================================

    ParameterValueDisplay.h
    Created: 17 Mar 2024 12:37:44am
    Author:  Arthur Wilson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../GraphicsGlobals.h"

class ParameterValueDisplay
: public juce::Component
, public juce::AudioProcessorParameter::Listener
{
public:
    enum Type { MacroParameter,  GlobalParameter };
    
    ParameterValueDisplay();
    ~ParameterValueDisplay();

    void resized() override;
    
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;
    
    void addParameter(juce::RangedAudioParameter* parameter);
    void setType(Type type);
    juce::String formattedValue();
    
private:
    juce::Label valueLabel;
    juce::ImageComponent backgroundImage;
    juce::RangedAudioParameter* param;
};
