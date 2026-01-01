/*
  ==============================================================================

    SwitchButton.h
    Created: 13 Mar 2024 11:13:56pm
    Author:  Arthur Wilson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SwitchButton
: public juce::Component
, public juce::AudioProcessorParameter::Listener
{
public:
    enum Title { MacroMap, Bypass, Atomic, Freeze };
    
    SwitchButton();
    ~SwitchButton();
    
    void setButtonText(Title buttonText);
    void addAttachment(juce::RangedAudioParameter* parameter);
    
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    void updateLEDState();
    
    juce::ImageComponent LEDon, LEDoff, textImage;
    juce::ImageButton button;
    std::unique_ptr<juce::ButtonParameterAttachment> attachment;
    juce::RangedAudioParameter* param;
};
