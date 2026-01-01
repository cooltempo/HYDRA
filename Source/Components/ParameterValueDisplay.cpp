/*
  ==============================================================================

    ParameterValueDisplay.cpp
    Created: 17 Mar 2024 12:37:44am
    Author:  Arthur Wilson

  ==============================================================================
*/

#include "ParameterValueDisplay.h"
#include "../MessageThreadUtils.h"

ParameterValueDisplay::ParameterValueDisplay()
{
    backgroundImage.setImage(juce::ImageCache::getFromMemory(BinaryData::KnobDisplayBg_png, BinaryData::KnobDisplayBg_pngSize));
    addChildComponent(backgroundImage);
    
    valueLabel.setJustificationType(juce::Justification::centredTop);
    valueLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colour(0xFF12F20D));
    valueLabel.setFont(getSubwayTickerFont());
    addAndMakeVisible(valueLabel);
}

ParameterValueDisplay::~ParameterValueDisplay()
{
    if (param)
        param->removeListener(this);
}

void ParameterValueDisplay::setType(Type type)
{
    if (type == GlobalParameter)
    {
        valueLabel.setJustificationType(juce::Justification::centred);
        backgroundImage.setVisible(true);
    }
}

void ParameterValueDisplay::resized()
{
    auto bounds = getLocalBounds();
    
    backgroundImage.setBounds(bounds);
    valueLabel.setBounds(bounds);
}

void ParameterValueDisplay::parameterValueChanged (int parameterIndex, float newValue)
{
    callSyncOrAsync([this] { valueLabel.setText(formattedValue(), juce::dontSendNotification); });
}

void ParameterValueDisplay::parameterGestureChanged (int parameterIndex, bool gestureIsStarting)
{
}

void ParameterValueDisplay::addParameter(juce::RangedAudioParameter* parameter)
{
    param = parameter;
    param->addListener(this);
    param = parameter;
    valueLabel.setText(formattedValue(), juce::dontSendNotification);
}

juce::String ParameterValueDisplay::formattedValue()
{
    if (param == nullptr)
        return "+0.00 " + param->getLabel();
    
    return param->getCurrentValueAsText() + " " + param->getLabel();
};
