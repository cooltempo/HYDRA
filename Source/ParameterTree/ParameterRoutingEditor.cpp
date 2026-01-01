/*
  ==============================================================================

    ParameterRoutingEditor.cpp
    Created: 8 Dec 2023 3:11:51pm
    Author:  Ed Powley

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ParameterRoutingEditor.h"

//==============================================================================
inline int packComboId(int a, int b) { return (a << 16) | (b + 1); }

inline std::pair<int, int> unpackComboId(int id) { return { id >> 16, (id & 0xFFFF) - 1 }; }

ParameterRoutingEditor::ParameterRoutingEditor(ParameterRouting& r,
                                               const juce::Array<ParameterRouting::Source*>& sources_)
: routing(r)
, sources(sources_)
, display(r)
{
    inRangeSlider.setSliderStyle(juce::Slider::TwoValueHorizontal);
    inRangeSlider.setRange(0.0, 1.0);
    inRangeSlider.setMinAndMaxValues(routing.getSourceMin(), routing.getSourceMax());
    inRangeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible(inRangeSlider);
    inRangeSlider.addListener(this);
    
    const int textBoxWidth = 75;
    
    outMinSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    outMinSlider.setRange(0.0, 1.0);
    outMinSlider.setValue(routing.getDestMin());
    outMinSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, textBoxWidth, 25);
    addAndMakeVisible(outMinSlider);
    outMinSlider.addListener(this);
    outMinSlider.onValueChange = [this] { display.refreshMappingCurve(); };
    
    outMaxSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    outMaxSlider.setRange(0.0, 1.0);
    outMaxSlider.setValue(routing.getDestMax());
    outMaxSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, textBoxWidth, 25);
    addAndMakeVisible(outMaxSlider);
    outMaxSlider.addListener(this);
    outMaxSlider.onValueChange = [this] { display.refreshMappingCurve(); };

    outSkewSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    outSkewSlider.setNormalisableRange(bsfx::getLogarithmicRange(0.1, 10.0));
    outSkewSlider.setValue(routing.getSkew());
    outSkewSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, textBoxWidth, 25);
    addAndMakeVisible(outSkewSlider);
    outSkewSlider.addListener(this);
    outSkewSlider.onValueChange = [this] { display.refreshMappingCurve(); };

    populateSourceCombo();
    addAndMakeVisible(sourceCombo);
    
    auto [am, featureIndex] = routing.getSource();
    int amIndex = sources.indexOf(am);
    if (amIndex != -1)
    {
        sourceCombo.setSelectedId(packComboId(amIndex, featureIndex));
    }
    
    sourceCombo.onChange = [this] { onSourceChange(); };
    
    averageTypeCombo.addItem("Raw value", 1);
    averageTypeCombo.addItem("Moving", 2);
    averageTypeCombo.addItem("Lifetime", 3);
    averageTypeCombo.setSelectedId((int)routing.getAverageType() + 1);
    averageTypeCombo.onChange = [this] {
        int id = averageTypeCombo.getSelectedId();
        if (id >= 1 && id <= 3)
        {
            routing.setAverageType((ParameterRouting::AverageType)(id - 1));
        }
    };
    addAndMakeVisible(averageTypeCombo);

    addAndMakeVisible(display);
    
    labels[0].setText("Source",     juce::dontSendNotification);
    labels[1].setText("Avg Type",   juce::dontSendNotification);
    labels[2].setText("In Range",   juce::dontSendNotification);
    labels[3].setText("Out Min",    juce::dontSendNotification);
    labels[4].setText("Out Max",    juce::dontSendNotification);
    labels[5].setText("Skew",       juce::dontSendNotification);

    for (auto& label : labels)
    {
        label.setFont(10);
        addAndMakeVisible(label);
    }

    onDestChange();
    
    setSize(600, 158);
}

ParameterRoutingEditor::~ParameterRoutingEditor()
{
    
}

//=============================================================================
void ParameterRoutingEditor::populateSourceCombo()
{
    juce::String arrow = juce::CharPointer_UTF8 (" \xe2\x86\x92 ");
    
    for (int sourceIndex = 0; sourceIndex < sources.size(); sourceIndex++)
    {
        auto source = sources[sourceIndex];
        
        for (int featureIndex = 0; featureIndex < source->getNumFeatures(); featureIndex++)
        {
            juce::String text = source->getSourceName();
            juce::String featureName = source->getFeatureName(featureIndex);
            if (featureName.isNotEmpty())
                text += arrow + featureName;
            
            sourceCombo.addItem(text, packComboId(sourceIndex, featureIndex));
        }
    }
}

//=============================================================================
void ParameterRoutingEditor::onSourceChange()
{
    int selectedId = sourceCombo.getSelectedId();
    if (selectedId != 0)
    {
        auto [modIndex, featureIndex] = unpackComboId(selectedId);
        
        jassert(modIndex >= 0 && modIndex < sources.size());
        auto mod = sources.getUnchecked(modIndex);
        
        jassert(featureIndex >= 0 && featureIndex < mod->getNumFeatures());
        
        routing.setSource(mod, featureIndex);
    }
    
    display.refreshMappingCurve();
}

void ParameterRoutingEditor::onDestChange()
{
    auto param = routing.getDest().second;
    
    auto setupSlider = [this, param] (juce::Slider& slider)
    {
        slider.textFromValueFunction = [param] (double v) {
            return param->getText((float)v, 1000) + " " + param->getLabel();
        };
        
        slider.valueFromTextFunction = [param] (const juce::String& s) {
            juce::String s2 = s;
            juce::String label = param->getLabel();
            if (label.isNotEmpty() && s2.endsWith(label))
                s2 = s2.substring(0, s2.length() - label.length());

            return param->getValueForText(s2);
        };
        
        slider.updateText();
    };
    
    setupSlider(outMinSlider);
    setupSlider(outMaxSlider);
    
    display.refreshMappingCurve();
}

void ParameterRoutingEditor::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &inRangeSlider)
    {
        routing.setSourceRange(inRangeSlider.getMinValue(), inRangeSlider.getMaxValue());
    }
    else if (slider == &outMinSlider || slider == &outMaxSlider)
    {
        routing.setDestRange(outMinSlider.getValue(), outMaxSlider.getValue());
    }
    else if (slider == &outSkewSlider)
    {
        routing.setSkew(outSkewSlider.getValue());
    }
    
    display.refreshMappingCurve();
}

//=============================================================================
void ParameterRoutingEditor::paint(juce::Graphics &g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

//    g.setColour (juce::Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
}

void ParameterRoutingEditor::resized()
{
    auto bounds = getLocalBounds(); //.reduced(4);
    
    display.setBounds(bounds.removeFromRight(bounds.getHeight()));
    bounds.removeFromRight(4);
    
    const int numControls = 6;
    int controlHeight = bounds.getHeight() / numControls;
    
    auto labelBounds = bounds.removeFromLeft(50).withTrimmedRight(4);
    
    for (auto& label : labels)
        label.setBounds(labelBounds.removeFromTop(controlHeight));
    
    sourceCombo.setBounds(bounds.removeFromTop(controlHeight));
    averageTypeCombo.setBounds(bounds.removeFromTop(controlHeight));
    inRangeSlider.setBounds(bounds.removeFromTop(controlHeight));
    outMinSlider.setBounds(bounds.removeFromTop(controlHeight));
    outMaxSlider.setBounds(bounds.removeFromTop(controlHeight));
    outSkewSlider.setBounds(bounds.removeFromTop(controlHeight));
}

//=============================================================================
ParameterRoutingEditor::ItemDisplay::ItemDisplay(ParameterRouting& r)
: routing(r)
{
    refreshMappingCurve();
    
    startTimerHz(30);
}

ParameterRoutingEditor::ItemDisplay::~ItemDisplay()
{
    
}

void ParameterRoutingEditor::ItemDisplay::resized()
{
    refreshMappingCurve();
}

//=============================================================================
void ParameterRoutingEditor::ItemDisplay::refreshMappingCurve()
{
    auto bounds = getLocalBounds();
    mappingCurve.clear();
    
    for (int x=0; x<bounds.getWidth(); x++)
    {
        float inValue = (float)x / (float)(bounds.getWidth() - 1);
        float outValue = routing.mapValue(inValue);
        
        float y = (1.0f - outValue) * (bounds.getHeight() - 1);
        
        if (x == 0)
            mappingCurve.startNewSubPath(x, y);
        else
            mappingCurve.lineTo(x, y);
    }
    
    repaint();
}

void ParameterRoutingEditor::ItemDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::yellow);
    g.strokePath(mappingCurve, juce::PathStrokeType(1.0f));

    auto bounds = getLocalBounds();
    auto [sourceValue, destValue] = routing.getLastValues();
    float x = sourceValue * (bounds.getWidth() - 1);
    float y = (1.0f - destValue) * (bounds.getHeight() - 1);
    g.setColour(juce::Colours::orange);
    g.drawEllipse(x-3, y-3, 6, 6, 1);
}
