/*
  ==============================================================================

    AnalysisModule.cpp
    Created: 7 Jan 2024 6:17:16pm
    Author:  Ed Powley

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AnalysisModuleItem.h"
#include "ParameterItem.h"
#include "ProcessorItem.h"

using namespace ParameterTreeItems;

AnalysisModuleComponent::AnalysisModuleComponent(AnalysisModule& p, const juce::String& titleString)
: analysisModule(p)
{
    title.setText(titleString.isNotEmpty() ? titleString : p.getName(), juce::dontSendNotification);
    addAndMakeVisible(title);
    
    for (int i=0; i<analysisModule.getNumFeatures(); i++)
    {
        AnalysisReadout* r = new AnalysisReadout(analysisModule, i);
        readouts.add(r);
        addAndMakeVisible(r);
    }
}

void AnalysisModuleComponent::resized()
{
    auto bounds = getLocalBounds();
    auto titleBounds = bounds.removeFromTop(c_titleHeight);
    title.setBounds(titleBounds);
    
    for (auto* r : readouts)
    {
        r->setBounds(bounds.removeFromTop(c_readoutHeight));
    }
}

//=============================================================================
AnalysisModuleItem::AnalysisModuleItem(AnalysisModule& p, const juce::String& title_)
: analysisModule(p), title(title_)
{
}

bool AnalysisModuleItem::mightContainSubItems()
{
    return !analysisModule.getParameters().isEmpty();
}

int AnalysisModuleItem::getItemHeight() const
{
    return AnalysisModuleComponent::c_titleHeight
        + analysisModule.getNumFeatures() * AnalysisModuleComponent::c_readoutHeight;
}

std::unique_ptr<juce::Component> AnalysisModuleItem::createItemComponent()
{
    return std::make_unique<AnalysisModuleComponent>(analysisModule, title);
}

void AnalysisModuleItem::itemOpennessChanged(bool isNowOpen)
{
    clearSubItems();
    
    if (isNowOpen)
        populateSubItems();
}
    
void AnalysisModuleItem::populateSubItems()
{
    auto customComponent = analysisModule.createCustomComponent();
    if (customComponent)
    {
        addSubItem(new ProcessorCustomItem(analysisModule, std::move(customComponent)));
    }

    for (auto param : analysisModule.getParameters())
    {
        addSubItem(new Parameter(analysisModule, param));
    }
}
