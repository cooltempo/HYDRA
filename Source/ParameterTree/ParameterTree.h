/*
  ==============================================================================

    ParameterTree.h
    Created: 13 Jun 2023 4:37:08pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../atomicengine/bsfx/Graph.h"
#include "../../atomicengine/Analysis/AnalysisModule.h"

//==============================================================================
/*
*/
class ParameterTree  : public juce::TreeView
{
public:
    ParameterTree();
    ~ParameterTree() override;
    
    void add(juce::AudioProcessor& proc, const juce::String& name = {});
    void add(AnalysisModule& am, const juce::String& name = {});

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterTree)
};


class ParameterTreeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    template<typename T>
    ParameterTreeAudioProcessorEditor(T& proc)
    : juce::AudioProcessorEditor(proc), tree(proc)
    {
        addAndMakeVisible(tree);
        setSize(600, 400);
    }
    
    void resized() override
    {
        tree.setBounds(getLocalBounds());
    }
    
    
private:
    ParameterTree tree;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterTreeAudioProcessorEditor)
};
