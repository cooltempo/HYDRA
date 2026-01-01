/*
  ==============================================================================

    Root.h
    Created: 7 Jan 2024 6:16:42pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

namespace ParameterTreeItems
{

class Root : public juce::TreeViewItem
{
public:
    bool mightContainSubItems() override { return true; }
    
};

}

