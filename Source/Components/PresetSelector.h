/*
  ==============================================================================

    PresetSelector.h
    Created: 18 Jan 2024 5:48:16pm
    Author:  Arthur Wilson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "GradientLabel.h"

class PresetSelector  : public juce::Component, public AtomicEngine::Listener
{
public:
    PresetSelector(HydraAudioProcessor& p);
    ~PresetSelector() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    void engineSelectedPresetChanged() override;
    void engineSelectedSnapshotChanged() override;

private:
    HydraAudioProcessor& processor;
    
    GradientLabel headingLabel[3];
    juce::Label nameLabel[3];
    std::unique_ptr<juce::Drawable> prevIcon, nextIcon;
    std::unique_ptr<juce::DrawableButton> prevButton[3];
    std::unique_ptr<juce::DrawableButton> nextButton[3];
    
    void updateDisplay();
    
    void handleButtonClick(int buttonIndex, int delta);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetSelector)
};
