/*
  ==============================================================================

    Parameter.h
    Created: 7 Jan 2024 6:16:31pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include "../atomicengine/ParameterRouting.h"

class AtomicEngine;
class ParameterRoutingEditor;
class AnalysisModule;

namespace ParameterTreeItems
{

class Parameter;

//==============================================================================
class ParameterComponent : public juce::Component
{
public:
    ParameterComponent(Parameter& item);
    
    void paint(juce::Graphics &g) override;
    void resized() override;
    
protected:
    Parameter& treeItem;
    
    virtual void layoutChildren(juce::Rectangle<int> bounds) = 0;
    juce::AudioProcessor& processor;
    juce::RangedAudioParameter* param;
    
    juce::Label nameLabel, idLabel, unitLabel;
    juce::TextButton routingButton;
    
    void onRoutingButtonClicked();
};

//==============================================================================
class ParameterSliderComponent : public ParameterComponent
{
public:
    ParameterSliderComponent(Parameter& item);
    
protected:
    void layoutChildren(juce::Rectangle<int> bounds) override;
    
private:
    juce::Slider slider;
    std::unique_ptr<juce::SliderParameterAttachment> sliderAttachment;
};

//==============================================================================
class ParameterButtonsComponent : public ParameterComponent, public juce::AudioProcessorParameter::Listener
{
public:
    ParameterButtonsComponent(Parameter& item);
    
    ~ParameterButtonsComponent() override;
    
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}
    
protected:
    void layoutChildren(juce::Rectangle<int> bounds) override;
    
private:
    juce::OwnedArray<juce::TextButton> buttons;
    
    void updateFromParameter();
};

//==============================================================================
class ParameterComboComponent : public ParameterComponent
{
public:
    ParameterComboComponent(Parameter& item);
    
protected:
    void layoutChildren(juce::Rectangle<int> bounds) override;
    
private:
    juce::ComboBox comboBox;
    std::unique_ptr<juce::ComboBoxParameterAttachment> comboAttachment;
};

//==============================================================================
class ParameterTextDisplayComponent : public ParameterComponent, public juce::AudioProcessorParameter::Listener
{
public:
    ParameterTextDisplayComponent(Parameter& item);
    ~ParameterTextDisplayComponent() override;
    
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}
    
protected:
    void layoutChildren(juce::Rectangle<int> bounds) override;
    
private:
    juce::Label label;
};

//==============================================================================
class ParameterRoutingComponent : public ParameterComponent
{
public:
    ParameterRoutingComponent(Parameter& item);
    
protected:
    void layoutChildren(juce::Rectangle<int> bounds) override;
    
private:
    std::unique_ptr<ParameterRoutingEditor> routingEditor;
    juce::Array<ParameterRouting::Source*> sources;
};

//==============================================================================
class Parameter : public juce::TreeViewItem
{
    friend class ParameterComponent;
    
public:
    Parameter(juce::AudioProcessor& proc, juce::AudioProcessorParameter* p);
    
    bool mightContainSubItems() override { return false; }
    
    std::unique_ptr<juce::Component> createItemComponent() override;
    
    int getItemHeight() const override;

    AtomicEngine* getEngine() const;
    
    ParameterRouting* getRouting() const;

private:
    juce::AudioProcessor& processor;
    juce::AudioProcessorParameter* param;
};

//==============================================================================
class ParameterColourGroup;

class ParameterColourGroupComponent 
: public juce::Component
, public juce::AudioProcessorParameter::Listener
, public juce::ChangeListener
{
public:
    ParameterColourGroupComponent(ParameterColourGroup& item);
    ~ParameterColourGroupComponent() override;
    
    void resized() override;
    
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}
    
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    
private:
    ParameterColourGroup& treeItem;
    
    juce::Label nameLabel;
    std::unique_ptr<juce::ColourSelector> colourSelector;
    
    void updateFromParameters();
};

//==============================================================================
class ParameterColourGroup : public juce::TreeViewItem
{
    friend class ParameterColourGroupComponent;
    
public:
    static bool isColourGroup(const juce::Array<juce::AudioProcessorParameter*> params, int index);
    
    ParameterColourGroup(juce::AudioProcessor& proc, int paramIndex, bool compact = true);

    bool mightContainSubItems() override { return isCompact; }
    void itemOpennessChanged(bool isNowOpen) override;
    
    std::unique_ptr<juce::Component> createItemComponent() override;
    int getItemHeight() const override { return isCompact ? 35 : 200; }

private:
    juce::AudioProcessor& processor;
    int firstParamIndex;
    const bool isCompact;
    juce::AudioParameterFloat* params[4];
    
    void populateSubItems();
};

//==============================================================================
} // end namespace ParameterTreeItems
