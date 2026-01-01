/*
  ==============================================================================

    HamburgerIcon.h
    Created: 7 May 2024 12:17:45pm
    Author:  Arthur Wilson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AnimatedIcon.h"

//==============================================================================
/*
*/
class HamburgerIcon  : public juce::Button
{
public:
    HamburgerIcon() : juce::Button("Hamburger Button")
    {
        createIcons();
        createGradients();
        animatedIcon.setIcon(hamburgerIcon);
        animatedIcon.setFadeSpeed(0.1f);
        addAndMakeVisible(animatedIcon);
        
        setClickingTogglesState(true);
    }
    
    ~HamburgerIcon() override
    {
    }
    
    void createIcons()
    {
        // Hamburger icon
        juce::Line<float> line1(0, 3, 55, 3);
        juce::Line<float> line2(0, 20, 55, 20);
        juce::Line<float> line3(0, 36, 55, 36);
        
        hamburgerIcon.addLineSegment(line1, 6.0f);
        hamburgerIcon.addLineSegment(line2, 6.0f);
        hamburgerIcon.addLineSegment(line3, 6.0f);
        
        // Arrow icon
        char const *arrow = "M54.2633 34.1224C54.259 38.0617 52.6923 41.8383 49.9068 44.6238C47.1213 47.4092 43.3447 48.976 39.4055 48.9803H18.9759C18.4833 48.9803 18.0109 48.7846 17.6626 48.4363C17.3143 48.088 17.1186 47.6156 17.1186 47.1231C17.1186 46.6305 17.3143 46.1581 17.6626 45.8098C18.0109 45.4615 18.4833 45.2658 18.9759 45.2658H39.4055C42.3609 45.2658 45.1952 44.0918 47.285 42.002C49.3748 39.9122 50.5489 37.0778 50.5489 34.1224C50.5489 31.167 49.3748 28.3326 47.285 26.2429C45.1952 24.1531 42.3609 22.979 39.4055 22.979H12.3154L20.2899 30.9512C20.6384 31.2997 20.8342 31.7724 20.8342 32.2652C20.8342 32.758 20.6384 33.2307 20.2899 33.5792C19.9414 33.9277 19.4687 34.1235 18.9759 34.1235C18.483 34.1235 18.0104 33.9277 17.6619 33.5792L6.51849 22.4358C6.34581 22.2633 6.20882 22.0585 6.11536 21.833C6.02189 21.6075 5.97379 21.3659 5.97379 21.1218C5.97379 20.8777 6.02189 20.636 6.11536 20.4106C6.20882 20.1851 6.34581 19.9803 6.51849 19.8078L17.6619 8.66439C18.0104 8.3159 18.483 8.12012 18.9759 8.12012C19.4687 8.12012 19.9414 8.3159 20.2899 8.66439C20.6384 9.01288 20.8342 9.48554 20.8342 9.97838C20.8342 10.4712 20.6384 10.9439 20.2899 11.2924L12.3154 19.2646H39.4055C43.3447 19.2689 47.1213 20.8356 49.9068 23.6211C52.6923 26.4065 54.259 30.1832 54.2633 34.1224Z";
        
        arrowIcon = juce::Drawable::parseSVGPath(arrow);
    }
    
    void createGradients()
    {
        juce::ColourGradient advanceGradient;
        advanceGradient.addColour(0.0, grey);
        advanceGradient.addColour(1.0, green);
        animatedIcon.setGradient(AnimatedIcon::State::Advance, advanceGradient);
        
        juce::ColourGradient retreatGradient;
        retreatGradient.addColour(0.0, grey);
        retreatGradient.addColour(1.0, green);
        animatedIcon.setGradient(AnimatedIcon::State::Retreat, retreatGradient);
    }
    
    void setState(AnimatedIcon::State state)
    {
        animatedIcon.setState(state);
    }
    
    void clicked() override
    {
        juce::Button::clicked();
        
        if (getToggleState())
            animatedIcon.setIcon(arrowIcon);
        else
            animatedIcon.setIcon(hamburgerIcon);
    }
    
    void paintButton (juce::Graphics& g,
                      bool shouldDrawButtonAsHighlighted,
                      bool shouldDrawButtonAsDown) override
    {
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        
        animatedIcon.setBounds(bounds.withSizeKeepingCentre(19, 13));
    }
    
private:
    AnimatedIcon animatedIcon;
    juce::Path hamburgerIcon, arrowIcon;
    juce::Colour grey{0xFFA1A6B2};
    juce::Colour green{0xFF12F20D};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HamburgerIcon)
};
