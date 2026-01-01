/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "Components/SwitchButton.h"
#include "Components/Knob.h"
#include "Components/PresetMenu.h"
#include "Components/PresetSelector.h"
#include "Components/VolumeMeter.h"
#include "Visualiser/Visualiser.h"

#include "Components/AtomicLogo.h"
#include "Components/HamburgerIcon.h"

#include "ParameterTree/ParameterTree.h"

//==============================================================================
class HydraAudioProcessorEditor
: public juce::Component
, public AtomicEngine::Listener
, public juce::Timer
{
public:
    HydraAudioProcessorEditor (HydraAudioProcessor&);
    ~HydraAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    
    //==============================================================================
    void engineMacroDisplayNameChanged(int macroIndex, juce::String name) override;
    
    HydraAudioProcessor* getAudioProcessor() { return &audioProcessor; }
    
private:
    HydraAudioProcessor& audioProcessor;
    
    juce::Image backgroundImage;
    SwitchButton macroMapButton, bypassButton, atomicButton, freezeButton;
    juce::OwnedArray<MacroKnob> macroKnobs;
    BottomKnob inputKnob, mixKnob, outputKnob;
    AtomicLogo atomicLogo;
    HamburgerIcon hamburgerIcon;
    juce::DrawableButton saveButton, settingsButton;
    PresetSelector presetSelector;
    ParameterTree parameterTree;
    HydraVolumeMeter inputMeter, outputMeter;
    Visualiser visualiser;
    PresetMenu presetMenu;

    std::unique_ptr<juce::AlertWindow> saveNameDialog;
    
#if HYDRA_ENABLE_DEV_MODE
    static const int c_devPanelWidth = 400;
    std::unique_ptr<juce::TabbedComponent> devPanel;
    juce::ImageButton devPanelButton;
    juce::Label visualiserFPS;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HydraAudioProcessorEditor)
};
