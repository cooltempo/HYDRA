/*
  ==============================================================================

    ParameterRoutingEditor.h
    Created: 8 Dec 2023 3:11:51pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../atomicengine/ParameterRouting.h"

//==============================================================================
/*
*/
class ParameterRoutingEditor  : public juce::Component, public juce::Slider::Listener
{
public:
    ParameterRoutingEditor(ParameterRouting& r, const juce::Array<ParameterRouting::Source*>& sources);
    ~ParameterRoutingEditor();
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void onSourceChange();
    void onDestChange();

    void sliderValueChanged(juce::Slider *slider) override;

private:
    ParameterRouting& routing;
    const juce::Array<ParameterRouting::Source*>& sources;

    juce::ComboBox sourceCombo, averageTypeCombo;
    juce::Slider inRangeSlider, outMinSlider, outMaxSlider, outSkewSlider;
    juce::Label labels[6];

    class ItemDisplay : public juce::Component, public juce::Timer
    {
    public:
        ItemDisplay(ParameterRouting& r);
        ~ItemDisplay();
        
        void paint (juce::Graphics&) override;
        void resized() override;
        
        void refreshMappingCurve();
        
        void timerCallback() override { repaint(); }
        
    private:
        ParameterRouting& routing;
        
        juce::Path mappingCurve;
                
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ItemDisplay);
    };
    
    ItemDisplay display;
    
    void populateSourceCombo();
        
    //=========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterRoutingEditor)
};
