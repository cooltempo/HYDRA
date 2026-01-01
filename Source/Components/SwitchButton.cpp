/*
  ==============================================================================

    SwitchButton.cpp
    Created: 13 Mar 2024 11:13:56pm
    Author:  Arthur Wilson

  ==============================================================================
*/

#include "SwitchButton.h"

SwitchButton::SwitchButton()
{
    button.setToggleable(true);
    button.setClickingTogglesState(true);
    
    LEDon.setImage(juce::ImageCache::getFromMemory(BinaryData::LEDon_png, BinaryData::LEDon_pngSize));
    addChildComponent(LEDon);
    
    LEDoff.setImage(juce::ImageCache::getFromMemory(BinaryData::LEDoff_png, BinaryData::LEDoff_pngSize));
    addAndMakeVisible(LEDoff);
    
    juce::Image buttonImage = juce::ImageCache::getFromMemory(BinaryData::Button_png, BinaryData::Button_pngSize);
    button.setImages(false, true, true,
                     buttonImage, 1.0f, juce::Colours::transparentBlack,
                     buttonImage, 1.0f, juce::Colours::transparentBlack,
                     buttonImage, 1.0f, juce::Colours::transparentBlack);
    addAndMakeVisible(button);
    
    addAndMakeVisible(textImage);
}

SwitchButton::~SwitchButton()
{
    if (param)
        param->removeListener(this);
}

void SwitchButton::setButtonText(Title buttonTitle)
{
    const char* imageData;
    int dataSize = 0;
    
    switch (buttonTitle) {
        case MacroMap:
            imageData = BinaryData::MacroMapText_png;
            dataSize = BinaryData::MacroMapText_pngSize;
            break;
        case Bypass:
            imageData = BinaryData::BypassText_png;
            dataSize = BinaryData::BypassText_pngSize;
            break;
        case Atomic:
            imageData = BinaryData::AtomicText_png;
            dataSize = BinaryData::AtomicText_pngSize;
            break;
        case Freeze:
            imageData = BinaryData::FreezeText_png;
            dataSize = BinaryData::FreezeText_pngSize;
            break;
    }
    
    textImage.setImage(juce::ImageCache::getFromMemory(imageData, dataSize));
    repaint();
}

void SwitchButton::addAttachment(juce::RangedAudioParameter* parameter)
{
    param = parameter;
    param->addListener(this);
    attachment = std::make_unique<juce::ButtonParameterAttachment>(*parameter, button);
    updateLEDState();
}

void SwitchButton::parameterValueChanged (int parameterIndex, float newValue)
{
    updateLEDState();
}

void SwitchButton::parameterGestureChanged (int parameterIndex, bool gestureIsStarting)
{
}

void SwitchButton::paint(juce::Graphics& g)
{
}

void SwitchButton::resized()
{
    auto bounds = getLocalBounds();
    
    auto LEDBounds = bounds.removeFromLeft(20);
    LEDon.setBounds(LEDBounds);
    LEDoff.setBounds(LEDBounds.withSizeKeepingCentre(8, 8));
    
    bounds.removeFromLeft(8);
    button.setBounds(bounds.removeFromLeft(50));
    bounds.removeFromLeft(10);
    textImage.setBounds(bounds);
}

void SwitchButton::updateLEDState()
{
    if (param == nullptr)
        return;
    
    bool state = param->getValue();
    LEDon.setVisible(state);
    LEDoff.setVisible(!state);
    repaint();
}
