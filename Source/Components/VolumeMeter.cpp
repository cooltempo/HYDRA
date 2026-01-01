/*
  ==============================================================================

    VolumeMeter.cpp
    Created: 18 Oct 2023 4:43:20pm

  ==============================================================================
*/

#include <JuceHeader.h>
#include "VolumeMeter.h"

#include "../../atomicengine/bsfx/Parameters.h"
#include "../GraphicsGlobals.h"

//==============================================================================
VolumeMeter::VolumeMeter()
{
    startTimerHz(60);
}

VolumeMeter::~VolumeMeter()
{
}

void VolumeMeter::setProcessorAndChannel(bsfx::VolumeMeter *processor_, int channel_)
{
    processor = processor_;
    channel = channel_;
    repaint();
}

void VolumeMeter::timerCallback()
{
    if (processor)
    {
        float value = processor->getMeterValue01(channel);
        value = juce::jlimit(0.0f, 1.0f, value);
        
        if (value != lastValue)
        {
            updateMeterValue(value);
            lastValue = value;
            repaint();
        }
    }
}

void VolumeMeter::updateMeterValue(float v)
{
    value = v;
}

void VolumeMeter::paint (juce::Graphics& g)
{
    g.setColour(meterGrey);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), getLocalBounds().toFloat().getWidth() * 0.5f);
    
    if (!isMeterOK() || isnan(value) || value < 0.0f || value > 1.0f)
    {
        g.fillAll(juce::Colours::red);
        return;
    }
    
    juce::Rectangle<float> bounds = getLocalBounds().toFloat().getProportion<float>({
        0.0f,
        1.0f - value,
        1.0f,
        value
    });

    if (bounds.getHeight() >= bounds.getWidth())
    {
        g.setGradientFill(juce::ColourGradient(red, 0, 0, green, getWidth(), getHeight(), false));
        g.fillRoundedRectangle(bounds, bounds.getWidth() * 0.5f);
    }
}

//==============================================================================
HydraVolumeMeter::HydraVolumeMeter()
{
    title.setFont(getSubwayTickerFont());
    title.setJustificationType(juce::Justification::centred);
    title.setColour(juce::Label::ColourIds::textColourId, orange.withAlpha(0.5f));
    addAndMakeVisible(title);
    
    level.setFont(getSubwayTickerFont().withPointHeight(7));
    level.setJustificationType(juce::Justification::centredLeft);
    level.setColour(juce::Label::ColourIds::textColourId, orange);
    addAndMakeVisible(level);
    
    db.setFont(getSubwayTickerFont().withPointHeight(7));
    db.setJustificationType(juce::Justification::centredLeft);
    db.setColour(juce::Label::ColourIds::textColourId, orange);
    db.setText("dB", juce::dontSendNotification);
    addAndMakeVisible(db);
    
    addAndMakeVisible(leftMeter);
    addAndMakeVisible(rightMeter);
    
    startTimerHz(60);
}
    
HydraVolumeMeter::~HydraVolumeMeter()
{
    stopTimer();
}

void HydraVolumeMeter::setTitle(juce::String string)
{
    title.setText(string, juce::dontSendNotification);
}

void HydraVolumeMeter::setProcessor(bsfx::VolumeMeter* p)
{
    processor = p;
    leftMeter.setProcessorAndChannel(processor, 0);
    rightMeter.setProcessorAndChannel(processor, 1);
}

void HydraVolumeMeter::paint(juce::Graphics& g)
{
}

void HydraVolumeMeter::resized()
{
    auto bounds = getLocalBounds();
    
    title.setBounds(bounds.removeFromTop(12));
    auto levelTextBounds = bounds.removeFromBottom(10);
    level.setBounds(levelTextBounds.removeFromLeft(30));
    db.setBounds(levelTextBounds);
    
    auto meterBounds = bounds.withSizeKeepingCentre(11, 320);
    leftMeter.setBounds(meterBounds.removeFromLeft(2));
    rightMeter.setBounds(meterBounds.removeFromRight(2));
}

void HydraVolumeMeter::timerCallback()
{
    float valueL, valueR, max;
    
    valueL = processor->getMeterValue(0);
    valueR = processor->getMeterValue(1);
    
    max = juce::jmax(valueL, valueR);
    
    juce::String string = bsfx::linearGainToDecibelString<2>(max, 6);
    
    if (string == "-∞")
        string = "-inf  ";
    else if (string == "0.00")
        string = "+0.00";
    
    level.setText(string, juce::dontSendNotification);
}

