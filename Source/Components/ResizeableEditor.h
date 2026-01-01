/*
  ==============================================================================

    ResizeableEditor.h
    Created: 30 Apr 2024 9:20:22pm
    Author:  Arthur Wilson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

//==============================================================================
class ResizeableEditor  : public juce::AudioProcessorEditor, public juce::ComponentListener
{
public:
    ResizeableEditor(HydraAudioProcessor& p, juce::Component* childEditor);
    ~ResizeableEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void componentMovedOrResized(juce::Component &component, bool wasMoved, bool wasResized) override;

private:
    HydraAudioProcessor& processor;
    std::unique_ptr<juce::Component> child;
    juce::Rectangle<int> childBounds;
    juce::ComponentBoundsConstrainer boundsConstrainer;
    
    void updateChildBounds();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizeableEditor)
};
