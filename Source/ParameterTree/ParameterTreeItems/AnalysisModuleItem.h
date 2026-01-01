/*
  ==============================================================================

    AnalysisModule.h
    Created: 7 Jan 2024 6:17:16pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include "../../atomicengine/Analysis/AnalysisModule.h"
#include "../AnalysisReadout.h"

namespace ParameterTreeItems
{

//==============================================================================
class AnalysisModuleComponent : public juce::Component
{
public:
    static constexpr int c_titleHeight = 20;
    static constexpr int c_readoutHeight = 30;
    
    AnalysisModuleComponent(AnalysisModule& p, const juce::String& titleString);
    
    void resized() override;
    
private:
    AnalysisModule& analysisModule;
    juce::OwnedArray<AnalysisReadout> readouts;

    juce::Label title;
};

//==============================================================================
class AnalysisModuleItem : public juce::TreeViewItem
{
public:
    AnalysisModuleItem(AnalysisModule& p, const juce::String& title_ = {});
    
    bool mightContainSubItems() override;
    
    int getItemHeight() const override;
    std::unique_ptr<juce::Component> createItemComponent() override;
    void itemOpennessChanged(bool isNowOpen) override;
        
private:
    AnalysisModule& analysisModule;
    juce::String title;
    
    void populateSubItems();
};

//==============================================================================
} // end namespace ParameterTreeItems

