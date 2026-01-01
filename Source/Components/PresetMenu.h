/*
  ==============================================================================

    PresetMenu.h
    Created: 8 May 2024 11:22:53pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GradientLabel.h"
#include "../PluginProcessor.h"

//==============================================================================
class PresetMenuLookAndFeel : public juce::LookAndFeel_V4
{
public:
    PresetMenuLookAndFeel();
    
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    
    void drawButtonBackground (juce::Graphics &g, juce::Button &button, const juce::Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
//    virtual void     drawButtonText (Graphics &, TextButton &, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)=0
    
    int getDefaultScrollbarWidth() override { return 5; }
    void drawScrollbar(juce::Graphics &, juce::ScrollBar &, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override;
    
    const juce::Font interRegularFont;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetMenuLookAndFeel)
};

//==============================================================================
class PresetMenu  : public juce::Component, public AtomicEngine::Listener
{
public:
    PresetMenu(HydraAudioProcessor& p);
    ~PresetMenu() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void engineSelectedPresetChanged() override;
    void engineSelectedSnapshotChanged() override;
    void engineSnapshotsChangedForPreset(const AtomicEngine::Preset &preset) override;
    void enginePresetFavouriteStateChanged(const AtomicEngine::Preset &preset) override;
    void engineSnapshotFavouriteStateChanged(const AtomicEngine::Snapshot &snapshot) override;
    
    static const juce::Colour c_gradientColours[6];
    
private:
    HydraAudioProcessor& processor;
    
    PresetMenuLookAndFeel laf;
    
    juce::TextEditor searchBox;
    std::unique_ptr<juce::Drawable> searchIcon;
    
    juce::TextButton filterButton[3];
    GradientLabel listHeading[3];
    juce::ListBox listBox[3];
    
    class ListRowComponent;
    class Model;
    class CategoriesModel;
    class PresetsModel;
    class SnapshotsModel;

    std::unique_ptr<CategoriesModel> categoriesModel;
    std::unique_ptr<PresetsModel> presetsModel;
    std::unique_ptr<SnapshotsModel> snapshotsModel;
    
    enum class FilterType { All, Suggested, Favourites };
    FilterType getCurrentFilterType();
    
    bool testFilter(const AtomicEngine::Preset* preset);
    bool testFilter(const AtomicEngine::Snapshot* snapshot);

    juce::Array<const AtomicEngine::Preset*> filteredPresets;
    juce::Array<const AtomicEngine::Snapshot*> filteredSnapshots;
    
    void populatePresetList();
    void populateSnapshotList();
    
    void updateSelection();
    bool isUpdatingSelection = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetMenu)
};
