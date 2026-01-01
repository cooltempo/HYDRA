/*
  ==============================================================================

    PresetSelector.cpp
    Created: 18 Jan 2024 5:48:16pm
    Author:  Arthur Wilson

  ==============================================================================
*/

#include "PresetSelector.h"
#include "../GraphicsGlobals.h"
#include "PresetMenu.h"

//==============================================================================
PresetSelector::PresetSelector(HydraAudioProcessor& p)
: processor(p)
{
    prevIcon = juce::Drawable::createFromImageData(BinaryData::PrevIcon_svg, BinaryData::PrevIcon_svgSize);
    nextIcon = juce::Drawable::createFromImageData(BinaryData::NextIcon_svg, BinaryData::NextIcon_svgSize);

    auto regularFont = getInterRegularFont().withPointHeight(10);
    auto mediumFont = getInterMediumFont();
    
    headingLabel[0].setText("Category", juce::dontSendNotification);
    headingLabel[1].setText("Preset", juce::dontSendNotification);
    headingLabel[2].setText("Snapshot", juce::dontSendNotification);

    for (int i=0; i<3; i++)
    {
        headingLabel[i].setFont(regularFont);
        headingLabel[i].setColour(GradientLabel::leftColourId, PresetMenu::c_gradientColours[i*2]);
        headingLabel[i].setColour(GradientLabel::rightColourId, PresetMenu::c_gradientColours[i*2+1]);
        headingLabel[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(headingLabel[i]);
        
        nameLabel[i].setFont(mediumFont);
        nameLabel[i].setColour(juce::Label::textColourId, juce::Colour(0xFFCDCCCC));
        nameLabel[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(nameLabel[i]);
        
        prevButton[i] = std::make_unique<juce::DrawableButton>("", juce::DrawableButton::ImageFitted);
        prevButton[i]->setImages(prevIcon.get());
        prevButton[i]->onClick = [this, i] { handleButtonClick(i, -1); };
        addAndMakeVisible(*prevButton[i]);

        nextButton[i] = std::make_unique<juce::DrawableButton>("", juce::DrawableButton::ImageFitted);
        nextButton[i]->setImages(nextIcon.get());
        nextButton[i]->onClick = [this, i] { handleButtonClick(i, +1); };
        addAndMakeVisible(*nextButton[i]);
    }
    
    updateDisplay();
    
    processor.engine->addListener(this);
}

PresetSelector::~PresetSelector()
{
    processor.engine->removeListener(this);
}

void PresetSelector::paint (juce::Graphics& g)
{

}

void PresetSelector::resized()
{
    auto bounds = getLocalBounds();
    int thirdWidth = bounds.getWidth() / 3;
    
    for (int i=0; i<3; i++)
    {
        auto panelBounds = bounds.removeFromLeft(thirdWidth).reduced(16, 9);
        prevButton[i]->setBounds(panelBounds.removeFromLeft(15).withSizeKeepingCentre(15, 15));
        nextButton[i]->setBounds(panelBounds.removeFromRight(15).withSizeKeepingCentre(15, 15));
        headingLabel[i].setBounds(panelBounds.removeFromTop(panelBounds.getHeight() / 2));
        nameLabel[i].setBounds(panelBounds);
    }
}

void PresetSelector::updateDisplay()
{
    const auto* preset = processor.engine->getSelectedPreset();
    if (preset)
    {
        nameLabel[0].setText(preset->category->name, juce::dontSendNotification);
        nameLabel[1].setText(preset->name, juce::dontSendNotification);
    }
    else
    {
        nameLabel[0].setText("-", juce::dontSendNotification);
        nameLabel[1].setText("-", juce::dontSendNotification);
    }
    
    const auto* snapshot = processor.engine->getSelectedSnapshot();
    if (snapshot)
    {
        nameLabel[2].setText(snapshot->name, juce::dontSendNotification);
    }
    else
    {
        nameLabel[2].setText("-", juce::dontSendNotification);
    }
}

void PresetSelector::engineSelectedPresetChanged()
{
    updateDisplay();
}

void PresetSelector::engineSelectedSnapshotChanged()
{
    updateDisplay();
}

void PresetSelector::handleButtonClick(int buttonIndex, int delta)
{
    switch (buttonIndex)
    {
        case 0:
            processor.engine->setSelectedPresetCategoryRelative(delta);
            break;

        case 1:
            processor.engine->setSelectedPresetRelative(delta);
            break;

        case 2:
            processor.engine->setSelectedSnapshotRelative(delta);
            break;
    }
}
