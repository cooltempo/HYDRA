/*
  ==============================================================================

    Processor.h
    Created: 7 Jan 2024 6:16:58pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include "../../../atomicengine/bsfx/Processor.h"

namespace ParameterTreeItems
{

//==============================================================================
class ProcessorCustomItem : public juce::TreeViewItem
{
public:
    ProcessorCustomItem(bsfx::Processor& p, std::unique_ptr<juce::Component> c);
    
    bool mightContainSubItems() override { return false; }
    
    int getItemHeight() const override;
    
    std::unique_ptr<juce::Component> createItemComponent() override;
    
private:
    bsfx::Processor& processor;
    mutable std::unique_ptr<juce::Component> component;
};


//==============================================================================
class Processor;

class ProcessorComponent : public juce::Component
{
public:
    ProcessorComponent(Processor& item);
    void resized() override;
    
private:
    Processor& treeItem;
    
    juce::AudioProcessor& processor;

    juce::Label title;
    juce::TextButton loadButton, saveButton;
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    void showFileChooser(bool isSave, std::function<void(const juce::File&)> callback);
    void loadPreset();
    void savePreset();
};

//==============================================================================
class Processor : public juce::TreeViewItem
{
    friend class ProcessorComponent;
    
public:
    Processor(juce::AudioProcessor& p, const juce::String& title_ = {});
    
    bool mightContainSubItems() override { return true; }
    
    std::unique_ptr<juce::Component> createItemComponent() override;
    
    void itemOpennessChanged(bool isNowOpen) override;
        
    AtomicEngine* getEngine() const;
    
private:
    juce::AudioProcessor& processor;
    juce::String title;
    
    void populateSubItems();
};

//==============================================================================
} // end namespace ParameterTreeItems

