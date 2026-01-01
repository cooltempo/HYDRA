/*
  ==============================================================================

    Parameter.cpp
    Created: 7 Jan 2024 6:16:31pm
    Author:  Ed Powley

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ParameterItem.h"
#include "../../../atomicengine/Analysis/AnalysisModule.h"
#include "../../../atomicengine/ParameterRouting.h"
#include "../../../atomicengine/bsfx/SetParameterValue.h"
#include "../ParameterRoutingEditor.h"
#include "../../MessageThreadUtils.h"
#include "../../PluginEditor.h"

using namespace ParameterTreeItems;

//==============================================================================
ParameterComponent::ParameterComponent(Parameter& item)
: treeItem(item), processor(item.processor)
{
    param = dynamic_cast<juce::RangedAudioParameter*>(treeItem.param);
    jassert(param != nullptr);
    
    nameLabel.setText(param->getName(1000), juce::dontSendNotification);
    nameLabel.setFont(12);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(nameLabel);
    
    idLabel.setText(param->getParameterID(), juce::dontSendNotification);
    idLabel.setFont(12);
    idLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFAAAAAA));
    idLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(idLabel);
    
    if (dynamic_cast<bsfx::Processor*>(&processor) != nullptr
        && dynamic_cast<AnalysisModule*>(&processor) == nullptr)
    {
        routingButton.setButtonText(item.getRouting() ? "Delete routing" : "Add routing");
        routingButton.onClick = [this] { onRoutingButtonClicked(); };
        addAndMakeVisible(routingButton);
    }
    
    unitLabel.setText(param->getLabel(), juce::dontSendNotification);
    addAndMakeVisible(unitLabel);
}
    
void ParameterComponent::paint(juce::Graphics &g)
{
}

void ParameterComponent::resized()
{
    auto bounds = getLocalBounds();
    
    auto labelBounds = bounds.removeFromTop(15); //.withTrimmedRight(50);
    
    if (routingButton.isVisible())
        routingButton.setBounds(labelBounds.removeFromRight(75));
    
    nameLabel.setBounds(labelBounds);
    idLabel.setBounds(labelBounds);
    
    if (unitLabel.isVisible())
    {
        unitLabel.setBounds(bounds.removeFromRight(50));
    }

    layoutChildren(bounds);
}

void ParameterComponent::onRoutingButtonClicked()
{
    auto* bsfxProc = dynamic_cast<bsfx::Processor*>(&processor);
    jassert(bsfxProc != nullptr);
    
    auto* currentRouting = bsfxProc->getParameterRouting(param->getParameterIndex());
    if (currentRouting)
    {
        bsfxProc->setParameterRouting(param->getParameterIndex(), nullptr);
    }
    else
    {
        auto* newRouting = new ParameterRouting;
        newRouting->setDest(bsfxProc, param);
        bsfxProc->setParameterRouting(param->getParameterIndex(), newRouting);
    }
    
    // Make parent recreate child items -- hacky but does the job for now
    treeItem.getParentItem()->itemOpennessChanged(true);
}

//==============================================================================
ParameterSliderComponent::ParameterSliderComponent(Parameter& item)
: ParameterComponent(item)
{
    slider.setScrollWheelEnabled(false);
    addAndMakeVisible(slider);
    sliderAttachment = std::make_unique<juce::SliderParameterAttachment>(*param, slider);
}

void ParameterSliderComponent::layoutChildren(juce::Rectangle<int> bounds)
{
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, bounds.getHeight());
    slider.setBounds(bounds);
}

//==============================================================================
ParameterButtonsComponent::ParameterButtonsComponent(Parameter& item)
: ParameterComponent(item)
{
    int numSteps = param->getNumSteps();
    for (int i=0; i<numSteps; i++)
    {
        juce::TextButton* button = new juce::TextButton;
        button->setButtonText(param->getText(param->convertTo0to1(i), 1000));
        button->setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFAAAACC));
        button->setColour(juce::TextButton::textColourOnId, juce::Colours::black);

        int connectedEdges = 0;
        if (i > 0)
            connectedEdges |= juce::Button::ConnectedOnLeft;
        if (i < numSteps-1)
            connectedEdges |= juce::Button::ConnectedOnRight;
        button->setConnectedEdges(connectedEdges);
        
        button->onClick = [this, i] {
            param->setValueNotifyingHost(param->convertTo0to1(i));
        };
        
        addAndMakeVisible(button);
        buttons.add(button);
    }
    
    updateFromParameter();
    param->addListener(this);
}

ParameterButtonsComponent::~ParameterButtonsComponent()
{
    param->removeListener(this);
}

void ParameterButtonsComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    callSyncOrAsync([this] { updateFromParameter(); });
}

void ParameterButtonsComponent::layoutChildren(juce::Rectangle<int> bounds)
{
    if (!buttons.isEmpty())
    {
        int buttonWidth = bounds.getWidth() / buttons.size();
        
        for (juce::Button* button : buttons)
        {
            button->setBounds(bounds.removeFromLeft(buttonWidth));
        }
    }
}

void ParameterButtonsComponent::updateFromParameter()
{
    int selectedIndex = juce::roundToInt(param->convertFrom0to1(param->getValue()));
    
    for (int i=0; i<buttons.size(); i++)
    {
        buttons[i]->setToggleState(i == selectedIndex, juce::dontSendNotification);
    }
}

//==============================================================================
ParameterComboComponent::ParameterComboComponent(Parameter& item)
: ParameterComponent(item)
{
    addAndMakeVisible(comboBox);
    
    int numSteps = param->getNumSteps();
    for (int i=0; i<numSteps; i++)
    {
        comboBox.addItem(param->getText(param->convertTo0to1(i), 1000), i+1);
    }
    
    comboAttachment = std::make_unique<juce::ComboBoxParameterAttachment>(*param, comboBox);
}

void ParameterComboComponent::layoutChildren(juce::Rectangle<int> bounds)
{
    comboBox.setBounds(bounds);
}

//==============================================================================
ParameterTextDisplayComponent::ParameterTextDisplayComponent(Parameter& item)
: ParameterComponent(item)
{
    label.setColour(juce::Label::outlineColourId, juce::Colours::grey);
    label.setJustificationType(juce::Justification::centred);
    label.setText(param->getCurrentValueAsText(), juce::dontSendNotification);
    addAndMakeVisible(label);

    param->addListener(this);
}

ParameterTextDisplayComponent::~ParameterTextDisplayComponent()
{
    param->removeListener(this);
}

void ParameterTextDisplayComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    callSyncOrAsync([this] {
        label.setText(param->getCurrentValueAsText(), juce::dontSendNotification);
    });
}

void ParameterTextDisplayComponent::layoutChildren(juce::Rectangle<int> bounds)
{
    label.setBounds(bounds);
}
    
//==============================================================================
ParameterRoutingComponent::ParameterRoutingComponent(Parameter& item)
: ParameterComponent(item)
{
    unitLabel.setVisible(false);
        
    item.getEngine()->forEachAnalysisModule([this](auto& am) {
        sources.add(&am);
    });
    item.getEngine()->forEachMacroParameter([this](auto& s) {
        sources.add(&s);
    });

    auto routing = item.getRouting();
    routingEditor = std::make_unique<ParameterRoutingEditor>(*routing, sources);
    addAndMakeVisible(*routingEditor);
}

void ParameterRoutingComponent::layoutChildren(juce::Rectangle<int> bounds)
{
    routingEditor->setBounds(bounds.withTrimmedLeft(10));
}

//==============================================================================
Parameter::Parameter(juce::AudioProcessor& proc, juce::AudioProcessorParameter* p)
: processor(proc), param(p)
{
    
}

ParameterRouting* Parameter::getRouting() const
{
    if (auto* bsfxProc = dynamic_cast<bsfx::Processor*>(&processor))
    {
        return bsfxProc->getParameterRouting(param->getParameterIndex());
    }
    else
    {
        return nullptr;
    }
}

int Parameter::getItemHeight() const
{
    if (getRouting())
        return 115;
    else
        return 35;
}

AtomicEngine* Parameter::getEngine() const
{
    // This is hacky, but we need to get access to the plugin's AtomicEngine instance
    // which we do by finding the editor window in the component hierarchy
    auto pluginEditor = getOwnerView()->findParentComponentOfClass<HydraAudioProcessorEditor>();
    auto plugin = pluginEditor->getAudioProcessor();
    jassert(plugin != nullptr);
    return plugin->engine;
}

std::unique_ptr<juce::Component> Parameter::createItemComponent()
{
    if (auto rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
    {
        bool paramIsReadOnly = !!(rangedParam->getCategory() & (2 << 16));
        if (getRouting())
            return std::make_unique<ParameterRoutingComponent>(*this);
        else if (paramIsReadOnly)
            return std::make_unique<ParameterTextDisplayComponent>(*this);
        else if (rangedParam->getNumSteps() <= 4)
            return std::make_unique<ParameterButtonsComponent>(*this);
        else if (dynamic_cast<juce::AudioParameterChoice*>(param))
            return std::make_unique<ParameterComboComponent>(*this);
        else
            return std::make_unique<ParameterSliderComponent>(*this);
    }
    else
    {
        jassertfalse;
        return nullptr;
    }
}

//==============================================================================
ParameterColourGroupComponent::ParameterColourGroupComponent(ParameterColourGroup& item)
: treeItem(item)
{
    if (item.isCompact)
    {
        juce::String name = item.params[0]->getName(1000);
        if (name.endsWith(" Red"))
            name = name.substring(0, name.length() - 4) + " Colour";
        
        nameLabel.setText(name, juce::dontSendNotification);
        nameLabel.setFont(12);
        nameLabel.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(nameLabel);

        colourSelector = std::make_unique<juce::ColourSelector>(juce::ColourSelector::editableColour |
                                                                juce::ColourSelector::showAlphaChannel |
                                                                juce::ColourSelector::showColourAtTop,
                                                                0, 0);
    }
    else
    {
        colourSelector = std::make_unique<juce::ColourSelector>(juce::ColourSelector::showAlphaChannel |
                                                                juce::ColourSelector::showColourspace,
                                                                4, 7);
    }
    
    addAndMakeVisible(*colourSelector);
    
    updateFromParameters();
    
    colourSelector->addChangeListener(this);
    
    for (auto* param : treeItem.params)
        param->addListener(this);
}

ParameterColourGroupComponent::~ParameterColourGroupComponent()
{
    for (auto* param : treeItem.params)
        param->removeListener(this);
}

void ParameterColourGroupComponent::resized()
{
    auto bounds = getLocalBounds();
    
    if (treeItem.isCompact)
    {
        auto labelBounds = bounds.removeFromTop(15);
        nameLabel.setBounds(labelBounds);

        // juce::ColourSelector is hard-coded to make the colour display take up no more than the top 20% of the component
        // We want it to take up the whole thing
        // Hacky solution: multiply the height by 5
        auto selectorBounds = bounds.withHeight(bounds.getHeight() * 5);
        colourSelector->setBounds(selectorBounds);
    }
    else
    {
        colourSelector->setBounds(bounds);
    }
}

void ParameterColourGroupComponent::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (source == colourSelector.get())
    {
        juce::Colour colour = colourSelector->getCurrentColour();
        bsfx::setParameterValue(treeItem.params[0], colour.getFloatRed());
        bsfx::setParameterValue(treeItem.params[1], colour.getFloatGreen());
        bsfx::setParameterValue(treeItem.params[2], colour.getFloatBlue());
        bsfx::setParameterValue(treeItem.params[3], colour.getFloatAlpha());
    }
}

void ParameterColourGroupComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    callSyncOrAsync([this] { updateFromParameters(); });
}

void ParameterColourGroupComponent::updateFromParameters()
{
    juce::Colour colour = juce::Colour::fromFloatRGBA(treeItem.params[0]->get(),
                                                      treeItem.params[1]->get(),
                                                      treeItem.params[2]->get(),
                                                      treeItem.params[3]->get());
    colourSelector->setCurrentColour(colour, juce::dontSendNotification);
}

//==============================================================================
bool ParameterColourGroup::isColourGroup(const juce::Array<juce::AudioProcessorParameter*> params, int index)
{
    if (index + 4 > params.size())
    {
        return false;
    }

    const juce::StringArray idSuffixes = { ".red", ".green", ".blue", ".alpha" };
    for (int i=0; i<4; i++)
    {
        auto* param = dynamic_cast<juce::AudioParameterFloat*>(params.getUnchecked(index + i));
        if (!param || !param->getParameterID().endsWith(idSuffixes.getReference(i)))
            return false;
    }
    
    return true;
}

ParameterColourGroup::ParameterColourGroup(juce::AudioProcessor& proc, int paramIndex, bool compact)
: processor(proc)
, firstParamIndex(paramIndex)
, isCompact(compact)
{
    const auto& ps = proc.getParameters();
    jassert(isColourGroup(ps, firstParamIndex));
    
    for (int i=0; i<4; i++)
    {
        params[i] = dynamic_cast<juce::AudioParameterFloat*>(ps.getUnchecked(firstParamIndex + i));
        jassert(params[i]);
    }
}

std::unique_ptr<juce::Component> ParameterColourGroup::createItemComponent()
{
    return std::make_unique<ParameterColourGroupComponent>(*this);
}

void ParameterColourGroup::itemOpennessChanged(bool isNowOpen)
{
    clearSubItems();
    
    if (isNowOpen)
        populateSubItems();
}
        
void ParameterColourGroup::populateSubItems()
{
    if (isCompact)
    {
        addSubItem(new ParameterColourGroup(processor, firstParamIndex, false));
        for (auto* param : params)
        {
            addSubItem(new Parameter(processor, param));
        }
    }
}
