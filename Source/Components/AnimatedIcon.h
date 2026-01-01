/*
  ==============================================================================

    AnimatedIcon.h
    Created: 7 May 2024 11:33:55am
    Author:  Arthur Wilson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class AnimatedIcon  : public juce::Component, public juce::Timer
{
public:
    enum State { Dead, Advance, Alive, Retreat };
    
    AnimatedIcon()
    {
        setInterceptsMouseClicks(false, false);
        currentGradient.addColour(0, juce::Colours::red);
        startTimer(30);
    }

    ~AnimatedIcon() override
    {
    }

    void setIcon(juce::Path p)
    {
        icon = p;
        resized();
        repaint();
    }
    
    void setGradient (State state, juce::ColourGradient gradient)
    {
        if (state == Advance)
            advanceGradient = gradient;
        else if (state == Retreat)
            retreatGradient = gradient;
        
        if (currentState == Dead || currentState == Advance)
        {
            currentGradient.clearColours();
            currentGradient = advanceGradient;
        }
        else if (currentState == Alive || currentState == Retreat)
        {
            currentGradient.clearColours();
            currentGradient = retreatGradient;
        }
    }
    
    void setFadeSpeed(float speed)
    {
        difference = speed;
    }
    
    void setState(State newState)
    {
        if (newState == currentState)
            return;
        
        
        if ((newState == Advance && currentState != Alive) ||
            (newState == Retreat && currentState != Dead))
        {
            updateCurrentGradient(newState);
            currentState = newState;
        }
    }
    
    State getState()
    {
        return currentState;
    }
    
    void updateCurrentGradient(State state)
    {
        juce::Colour currentColour = currentGradient.getColourAtPosition(colourPos);
        currentGradient.clearColours();
        
        if (state == Advance)
            currentGradient = advanceGradient;
        else if (state == Retreat)
            currentGradient = retreatGradient;
        
        currentGradient.addColour(colourPos, currentColour);
        startTimer(100);
    }
    
    void paint (juce::Graphics& g) override
    {
        juce::Colour currColour = currentGradient.getColourAtPosition(colourPos);
        
        juce::DropShadow dropShadow(currColour, 6, {0, 0});
        dropShadow.drawForPath(g, icon);
        
        g.setColour(currColour);
        g.fillPath(icon);
    }

    void resized() override
    {
        icon.scaleToFit(0, 0, getWidth(), getHeight(), true);
    }
    
    void timerCallback() override
    {
        switch (currentState)
        {
            case Dead:
            case Alive:
                return;
            case Advance:
                colourPos += difference;
                break;
            case Retreat:
                colourPos -= difference;
                break;
        }
        
        if (colourPos >= 1.0f)
        {
            colourPos = 1.0f;
            currentState = Alive;
            stopTimer();
        }
        else if (colourPos <= 0.0f)
        {
            colourPos = 0.0f;
            currentState = Dead;
            stopTimer();
        }

        repaint();
    }

private:
    State currentState = Dead;
    float colourPos = 0.0f;
    float difference = 0.02f;
    juce::Path icon;
    
    juce::ColourGradient advanceGradient, retreatGradient, currentGradient;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnimatedIcon)
};
