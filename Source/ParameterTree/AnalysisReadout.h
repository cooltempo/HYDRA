/*
  ==============================================================================

    AnalysisReadout.h
    Created: 27 Nov 2023 4:34:06pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "../../atomicengine/Analysis/AnalysisModule.h"

//==============================================================================
/*
*/
class AnalysisReadout  : public juce::Component, public juce::Timer
{
public:
    AnalysisReadout(AnalysisModule& m, int featureIndex_)
    : analysisModule(m), featureIndex(featureIndex_)
    {
        featureName = analysisModule.getFeatureName(featureIndex);
        startTimerHz(60);
    }

    ~AnalysisReadout() override
    {
    }
    
    void timerCallback() override
    {
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        
        float movingAverageValue = analysisModule.getMovingAverageValue(featureIndex);
        g.setColour(juce::Colours::darkgreen);
        auto rect = getLocalBounds().toFloat();
        rect = rect.getProportion<float>({0, 0, movingAverageValue, 0.5f});
        g.fillRect(rect);
        
        float lifetimeAverageValue = analysisModule.getLifetimeAverageValue(featureIndex);
        g.setColour(juce::Colours::blue);
        rect = getLocalBounds().toFloat();
        rect = rect.getProportion<float>({0, 0.5f, lifetimeAverageValue, 0.5f});
        g.fillRect(rect);

        float lastValue = analysisModule.getLastValue(featureIndex);
        g.setColour(juce::Colours::yellowgreen);
        float x = getWidth() * lastValue;
        g.drawLine(x, 0.0f, x, getHeight());

        g.setColour (juce::Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setFont (14.0f);
        if (!featureName.isEmpty())
        {
            g.setColour(juce::Colours::grey);
            g.drawText(featureName, getLocalBounds().withTrimmedLeft(4),
                       juce::Justification::centredLeft, true);
        }

        g.setColour (juce::Colours::white);
        juce::String text = juce::String::formatted("exp avg = %.3f    life avg = %.3f    now = %.3f",
                                                    movingAverageValue, lifetimeAverageValue, lastValue);
        g.drawText (text, getLocalBounds(),
                    juce::Justification::centred, true);
        
    }

    void resized() override
    {
    }

private:
    AnalysisModule& analysisModule;
    const int featureIndex;
    juce::String featureName;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnalysisReadout)
};
