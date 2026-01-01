/*
  ==============================================================================

    Processor.cpp
    Created: 7 Jan 2024 6:16:58pm
    Author:  Ed Powley

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ProcessorItem.h"
#include "ParameterItem.h"
#include "../../PluginEditor.h"

using namespace ParameterTreeItems;

//==============================================================================
ProcessorCustomItem::ProcessorCustomItem(bsfx::Processor& p, std::unique_ptr<juce::Component> c)
: processor(p), component(std::move(c))
{
}

int ProcessorCustomItem::getItemHeight() const
{
    if (!component)
        component = processor.createCustomComponent();
    
    return component->getHeight();
}

std::unique_ptr<juce::Component> ProcessorCustomItem::createItemComponent()
{
    if (!component)
        component = processor.createCustomComponent();
    
    return std::move(component);
}

//==============================================================================
ProcessorComponent::ProcessorComponent(Processor& item)
: treeItem(item)
, processor(item.processor)
{
    title.setText(treeItem.title.isNotEmpty() ? treeItem.title : processor.getName(), juce::dontSendNotification);
    addAndMakeVisible(title);
    
    if (dynamic_cast<bsfx::Processor*>(&processor))
    {
        loadButton.setButtonText("Load");
        loadButton.onClick = [this] { loadPreset(); };
        addAndMakeVisible(loadButton);
        
        saveButton.setButtonText("Save");
        saveButton.onClick = [this] { savePreset(); };
        addAndMakeVisible(saveButton);
    }
}

void ProcessorComponent::resized() 
{
    auto bounds = getLocalBounds();
    
    if (saveButton.isVisible())
        saveButton.setBounds(bounds.removeFromRight(50));
    
    if (loadButton.isVisible())
        loadButton.setBounds(bounds.removeFromRight(50));
    
    title.setBounds(bounds);
}

void ProcessorComponent::showFileChooser(bool isSave, std::function<void (const juce::File &)> callback)
{
    fileChooser = std::make_unique<juce::FileChooser>(isSave ? "Save Preset" : "Load Preset",    // dialogBoxTitle
                                                      juce::File(),     // initialFileOrDirectory
                                                      "*.xml",          // filePatternsAllowed
                                                      true,             // useOSNativeDialogBox
                                                      true,             // treatFilePackagesAsDirectories
                                                      this              // parentComponent
                                                      );
    
    int fileChooserFlags = juce::FileBrowserComponent::canSelectFiles;
    if (isSave)
        fileChooserFlags |= juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting;
    else
        fileChooserFlags |= juce::FileBrowserComponent::openMode;
    
    fileChooser->launchAsync(fileChooserFlags,
                             [this, callback](const juce::FileChooser& chooser) {
        
        juce::File file = chooser.getResult();
        if (file != juce::File())
        {
            DBG(file.getFullPathName());

            callback(file);
        }
        else
        {
            DBG("Cancelled");
        }
    });

}

void ProcessorComponent::loadPreset()
{
    showFileChooser(false, [this](const juce::File& file) {
        if (auto* bsProc = dynamic_cast<bsfx::Processor*>(&processor))
        {
            std::unique_ptr<juce::XmlElement> xml = juce::XmlDocument::parse(file);
            bsProc->loadPresetXml(treeItem.getEngine(), xml.get());
        }
    });
}
    
void ProcessorComponent::savePreset()
{
    showFileChooser(true, [this](const juce::File& file) {
        if (auto* bsProc = dynamic_cast<bsfx::Processor*>(&processor))
        {
            std::unique_ptr<juce::XmlElement> xml = bsProc->getPresetXml();
            xml->writeTo(file);
        }
    });
}

//==============================================================================
Processor::Processor(juce::AudioProcessor& p, const juce::String& title_)
: processor(p), title(title_)
{
    
}

std::unique_ptr<juce::Component> Processor::createItemComponent()
{
    return std::make_unique<ProcessorComponent>(*this);
}

AtomicEngine* Processor::getEngine() const
{
    // This is hacky, but we need to get access to the plugin's AtomicEngine instance
    // which we do by finding the editor window in the component hierarchy
    auto pluginEditor = getOwnerView()->findParentComponentOfClass<HydraAudioProcessorEditor>();
    auto plugin = pluginEditor->getAudioProcessor();
    jassert(plugin != nullptr);
    return plugin->engine;
}

void Processor::itemOpennessChanged(bool isNowOpen)
{
    clearSubItems();
    
    if (isNowOpen)
        populateSubItems();
}
        
void Processor::populateSubItems()
{
    if (auto bsfxProcessor = dynamic_cast<bsfx::Processor*>(&processor))
    {
        auto customComponent = bsfxProcessor->createCustomComponent();
        if (customComponent)
        {
            addSubItem(new ProcessorCustomItem(*bsfxProcessor, std::move(customComponent)));
        }
    }
    
    const auto& parameters = processor.getParameters();
    
    for (int i=0; i<parameters.size(); i++)
    {
        if (ParameterColourGroup::isColourGroup(parameters, i))
        {
            addSubItem(new ParameterColourGroup(processor, i));
            i += 3;
        }
        else
        {
            addSubItem(new Parameter(processor, parameters.getUnchecked(i)));
        }
    }
}
