/*
  ==============================================================================

    ParameterTree.cpp
    Created: 13 Jun 2023 4:37:08pm
    Author:  Ed Powley

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ParameterTree.h"

#include "./ParameterTreeItems/RootItem.h"
#include "./ParameterTreeItems/ProcessorItem.h"
#include "./ParameterTreeItems/AnalysisModuleItem.h"

//==============================================================================
ParameterTree::ParameterTree()
{
    ParameterTreeItems::Root* root = new ParameterTreeItems::Root;
    setRootItem(root);
    setRootItemVisible(false);
}

ParameterTree::~ParameterTree()
{
    deleteRootItem();
}

void ParameterTree::add(juce::AudioProcessor &proc, const juce::String& name)
{
    getRootItem()->addSubItem(new ParameterTreeItems::Processor(proc, name));
}

void ParameterTree::add(AnalysisModule &am, const juce::String& name)
{
    getRootItem()->addSubItem(new ParameterTreeItems::AnalysisModuleItem(am, name));
}
