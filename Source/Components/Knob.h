/*
  ==============================================================================

    Knob.h
    Created: 14 Mar 2024 1:58:18pm
    Author:  Arthur Wilson

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "ParameterValueDisplay.h"
#include "../GraphicsGlobals.h"

class HydraLEDSliderLAF
: public juce::LookAndFeel_V4
{
public:
    HydraLEDSliderLAF()
    {
    }
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
    {
        juce::Colour activeLedColour = sliderPos != 0.0f ? green : knobLEDBlack;
        
        // Knob crown LEDS
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (2);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto lineW = 2.0f;
        auto arcRadius = radius - lineW * 0.5f;
        float numLEDS = 48.0f;

        juce::Path backgroundArc;
        backgroundArc.addCentredArc (bounds.getCentreX(),
                                     bounds.getCentreY(),
                                     arcRadius,
                                     arcRadius,
                                     0.0f,
                                     rotaryStartAngle,
                                     rotaryEndAngle,
                                     true);
        
        float arcLength = backgroundArc.getLength();
        
        for (float i = 0; i < arcLength; i += arcLength / numLEDS)
        {
            auto p = backgroundArc.getPointAlongPath(i);
            
            float normI = i / arcLength;
            if (normI > sliderPos)
                g.setColour(knobLEDBlack);
            else
                g.setColour(activeLedColour);
                
            g.fillEllipse(p.x-lineW/2, p.y-lineW/2, lineW, lineW);
        }
        
        // Knob Thumb LED
        auto thumbBounds = bounds.reduced(18);
        auto thummbRadius = juce::jmin (thumbBounds.getWidth(), thumbBounds.getHeight()) / 2.0f;
        auto thumbArcRadius = thummbRadius - lineW * 0.5f;
        
        juce::Path thumbArc;
        thumbArc.addCentredArc (thumbBounds.getCentreX(),
                                thumbBounds.getCentreY(),
                                thumbArcRadius,
                                thumbArcRadius,
                                0.0f,
                                rotaryStartAngle,
                                rotaryEndAngle,
                                true);
        
        float thumbArcLength = thumbArc.getLength();
        float pos = thumbArcLength * sliderPos;
        auto p = thumbArc.getPointAlongPath(pos);
        
        g.setColour(activeLedColour);
        g.fillEllipse(p.x-lineW/2, p.y-lineW/2, lineW, lineW);
    }
};

class Knob
: public juce::Component
, public juce::AudioProcessorParameter::Listener
{
public:
    Knob();
    ~Knob();
    
    void addAttachment(juce::RangedAudioParameter* parameter);
    
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
private:
    juce::ImageComponent knob;
    juce::Slider slider;
    HydraLEDSliderLAF LAF;
    std::unique_ptr<juce::SliderParameterAttachment> attachment;
    juce::RangedAudioParameter* param;
};

//==============================================================================
class BottomKnob
: public juce::Component
{
public:
    enum Function { Input, Mix, Output };
    
    BottomKnob();
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void setText(Function function);
    void addAttachment(juce::RangedAudioParameter* parameter);

private:
    ParameterValueDisplay display;
    Knob knob;
    juce::ImageComponent titleImage;
};

//==============================================================================
class MacroKnob
: public juce::Component
{
public:
    MacroKnob();
    
    void resized() override;
    void paint(juce::Graphics& g) override;
    
    void setDisplayName(juce::String string);
    void addAttachment(juce::RangedAudioParameter* parameter);
    void setLeft(bool _isLeft);

private:
    ParameterValueDisplay display;
    Knob knob;
    juce::Label displayNameLabel;
    
    bool isLeft = true;
};
