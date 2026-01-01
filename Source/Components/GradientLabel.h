/*
  ==============================================================================

    GradientLabel.h
    Created: 9 May 2024 12:30:20am
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class GradientLabel : public juce::Label
{
public:
    GradientLabel() {}
    
    enum ColourIds
    {
        leftColourId = 0xB51000,
        rightColourId
    };
    
    void paint(juce::Graphics &g) override
    {
        auto bounds = getLocalBounds();
        g.setFont(getLookAndFeel().getLabelFont(*this));

        auto gradient = juce::ColourGradient::horizontal(findColour(leftColourId), 0,
                                                         findColour(rightColourId), bounds.getWidth());
        g.setGradientFill(gradient);
        
        g.drawText(getText(), bounds, getJustificationType());
    }
    
  
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GradientLabel)
};
