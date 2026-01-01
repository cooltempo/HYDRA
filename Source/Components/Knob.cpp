/*
  ==============================================================================

    Knob.cpp
    Created: 14 Mar 2024 1:58:18pm
    Author:  Arthur Wilson

  ==============================================================================
*/

#include "Knob.h"

Knob::Knob()
{
    setLookAndFeel(&LAF);
    
    knob.setImage(juce::ImageCache::getFromMemory(BinaryData::Knob_png, BinaryData::Knob_pngSize));
    addAndMakeVisible(knob);
    
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(slider);
}

Knob::~Knob()
{
    setLookAndFeel(nullptr);
    if (param)
        param->removeListener(this);
}

void Knob::addAttachment(juce::RangedAudioParameter* parameter)
{
    param = parameter;
    param->addListener(this);
    attachment = std::make_unique<juce::SliderParameterAttachment>(*parameter, slider);
}

void Knob::parameterValueChanged (int parameterIndex, float newValue)
{
    
}

void Knob::parameterGestureChanged (int parameterIndex, bool gestureIsStarting)
{
    
}

void Knob::resized()
{
    auto bounds = getLocalBounds();
    
    knob.setBounds(bounds);
    slider.setBounds(bounds.withSizeKeepingCentre(84, 84));
}

void Knob::paint(juce::Graphics& g)
{
}

//==============================================================================
BottomKnob::BottomKnob()
{
    display.setType(ParameterValueDisplay::Type::GlobalParameter);
    
    addAndMakeVisible(display);
    addAndMakeVisible(knob);
    addAndMakeVisible(titleImage);
}

void BottomKnob::resized()
{
    auto bounds = getLocalBounds();
    
    display.setBounds(bounds.removeFromTop(35));
    bounds.removeFromTop(15);
    titleImage.setBounds(bounds.removeFromBottom(16));
    knob.setBounds(bounds);
}

void BottomKnob::paint(juce::Graphics& g)
{
}

void BottomKnob::setText(Function function)
{
    switch (function)
    {
        case Input:
            titleImage.setImage(juce::ImageCache::getFromMemory(BinaryData::InputText_png, BinaryData::InputText_pngSize));
            break;
        case Mix:
            titleImage.setImage(juce::ImageCache::getFromMemory(BinaryData::MixText_png, BinaryData::MixText_pngSize));
            break;
        case Output:
            titleImage.setImage(juce::ImageCache::getFromMemory(BinaryData::OutputText_png, BinaryData::OutputText_pngSize));
            break;
    }
}

void BottomKnob::addAttachment(juce::RangedAudioParameter* parameter)
{
    knob.addAttachment(parameter);
    display.addParameter(parameter);
}

//==============================================================================
MacroKnob::MacroKnob()
{
    display.setType(ParameterValueDisplay::Type::MacroParameter);
    
    displayNameLabel.setFont(getInterSemiboldFont());
    displayNameLabel.setColour(juce::Label::ColourIds::textColourId, grey2);
    displayNameLabel.setJustificationType(juce::Justification::centredBottom);
    displayNameLabel.setMinimumHorizontalScale(1.0f);
    
    addAndMakeVisible(knob);
    addAndMakeVisible(display);
    addAndMakeVisible(displayNameLabel);
}
    
void MacroKnob::resized()
{
    auto bounds = getLocalBounds();
    juce::Rectangle<int> displayBounds;
    
    if (isLeft)
    {
        knob.setBounds(bounds.removeFromLeft(94));
        displayBounds = bounds.removeFromRight(70);
    }
    else
    {
        knob.setBounds(bounds.removeFromRight(94));
        displayBounds = bounds.removeFromLeft(70);
    }
    
    display.setBounds(displayBounds.removeFromBottom(50));
    displayNameLabel.setBounds(displayBounds);
    
}
void MacroKnob::paint(juce::Graphics& g)
{
}

void MacroKnob::setDisplayName(juce::String string)
{
    displayNameLabel.setText(string, juce::dontSendNotification);
}

void MacroKnob::addAttachment(juce::RangedAudioParameter* parameter)
{
    knob.addAttachment(parameter);
    display.addParameter(parameter);
}

void MacroKnob::setLeft(bool _isLeft)
{
    isLeft = _isLeft;
}
