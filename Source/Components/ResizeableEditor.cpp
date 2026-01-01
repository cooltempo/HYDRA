/*
  ==============================================================================

    ResizeableEditor.cpp
    Created: 30 Apr 2024 9:20:22pm
    Author:  Arthur Wilson

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ResizeableEditor.h"

//==============================================================================
ResizeableEditor::ResizeableEditor(HydraAudioProcessor& p, juce::Component* childEditor)
: juce::AudioProcessorEditor(p)
, processor(p)
{
    child.reset(childEditor);
    addAndMakeVisible(child.get());
    
    updateChildBounds();
    
    setSize(processor.editorWindow.getWidth(), processor.editorWindow.getHeight());
    
    child->addComponentListener(this);
}

ResizeableEditor::~ResizeableEditor()
{
    auto bounds = getLocalBounds();
    juce::Rectangle<int> newEditorWindow(bounds.getWidth(), bounds.getHeight());
    processor.editorWindow = newEditorWindow;
}

void ResizeableEditor::paint (juce::Graphics& g)
{
}

void ResizeableEditor::resized()
{
    auto bounds = getLocalBounds();
    
    float scaleFactorWidth = (float)bounds.getWidth() / (float)childBounds.getWidth();
    float scaleFactorHeight = (float)bounds.getHeight() / (float)childBounds.getHeight();
    
    float scaleFactor = juce::jmin(scaleFactorWidth, scaleFactorHeight);
    child->setTransform(juce::AffineTransform::scale(scaleFactor));
}

void ResizeableEditor::componentMovedOrResized(juce::Component &component, bool wasMoved, bool wasResized)
{
    if (&component == child.get() && wasResized)
    {
        float scaleFactorWidth = (float)getWidth() / (float)childBounds.getWidth();
        float scaleFactorHeight = (float)getHeight() / (float)childBounds.getHeight();
        float scaleFactor = juce::jmin(scaleFactorWidth, scaleFactorHeight);

        updateChildBounds();
        
        int newWidth = juce::roundToInt(childBounds.getWidth() * scaleFactor);
        int newHeight = juce::roundToInt(childBounds.getHeight() * scaleFactor);
        
        setSize(newWidth, newHeight);
    }
}

void ResizeableEditor::updateChildBounds()
{
    childBounds = child->getLocalBounds();
    
    int childWidth = childBounds.getWidth();
    int childHeight = childBounds.getHeight();
    float ratio = childWidth / (float) childHeight;
    
    boundsConstrainer.setFixedAspectRatio(ratio);
    boundsConstrainer.setSizeLimits(childWidth / 2, childHeight / 2, childWidth * 1.5, childHeight * 1.5);
    setConstrainer(&boundsConstrainer);
    setResizable(true, true);
}
