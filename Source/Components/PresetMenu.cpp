/*
  ==============================================================================

    PresetMenu.cpp
    Created: 8 May 2024 11:22:53pm
    Author:  Ed Powley

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PresetMenu.h"
#include "../GraphicsGlobals.h"

//==============================================================================
const juce::Colour PresetMenu::c_gradientColours[6] {
    juce::Colour(0xFFA600FF), juce::Colour(0xFF86DBFF),
    juce::Colour(0xFFFF5C00), juce::Colour(0xFFFEFEFE),
    juce::Colour(0xFF80FF00), juce::Colour(0xFF86FFC5),
};

//==============================================================================
class FavouriteButton : public juce::Button
{
public:
    FavouriteButton() : juce::Button("")
    {
        starPath = juce::Drawable::parseSVGPath("M16.0441 2.16485L18.349 6.48317C18.6633 7.08429 19.5015 7.66089 20.2087 7.7713L24.3864 8.4215C27.058 8.83861 27.6866 10.6543 25.7615 12.4454L22.5136 15.4878C21.9636 16.0031 21.6624 16.9968 21.8326 17.7083L22.7625 21.4746C23.4958 24.4557 21.8064 25.6089 18.9907 24.0508L15.075 21.8794C14.3678 21.4868 13.2022 21.4868 12.4819 21.8794L8.56613 24.0508C5.76354 25.6089 4.06103 24.4434 4.79442 21.4746L5.72425 17.7083C5.8945 16.9968 5.59329 16.0031 5.04324 15.4878L1.79538 12.4454C-0.116675 10.6543 0.498849 8.83861 3.17048 8.4215L7.34818 7.7713C8.04228 7.66089 8.88044 7.08429 9.19475 6.48317L11.4997 2.16485C12.7569 -0.178323 14.7999 -0.178323 16.0441 2.16485Z");
    }
    
    void setGradientIndex(int i)
    {
        gradientIndex = i;
    }
    
    void paintButton(juce::Graphics &g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        if (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown)
            g.fillAll(juce::Colours::white.withAlpha(shouldDrawButtonAsDown ? 0.4f : 0.2f));
        
        if (getToggleState())
            g.setGradientFill(starGradient);
        else
            g.setColour(juce::Colours::black);
        
        g.fillPath(starPath, starTransform);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().toFloat();
        auto starBounds = starPath.getBounds() * 0.25f;
        starTransform = (juce::AffineTransform::scale(0.25f)
                         .translated(bounds.getCentre() - starBounds.getCentre())
                         );
        
        starBounds = starPath.getBoundsTransformed(starTransform);
        starGradient = juce::ColourGradient::horizontal(PresetMenu::c_gradientColours[gradientIndex], starBounds.getX(),
                                                        PresetMenu::c_gradientColours[gradientIndex+1], starBounds.getRight());
    }
    
private:
    juce::Path starPath;
    juce::AffineTransform starTransform;
    juce::ColourGradient starGradient;
    int gradientIndex = 0;
};

//==============================================================================
class PresetMenu::ListRowComponent : public juce::Component
{
public:
    ListRowComponent(const juce::String& name, int index, int gradientIndex, bool isFavouritable)
    {
        setInterceptsMouseClicks(false, true);
        
        gradientColour[0] = PresetMenu::c_gradientColours[gradientIndex];
        gradientColour[1] = PresetMenu::c_gradientColours[gradientIndex + 1];
        
        nameLabel.setText(name, juce::dontSendNotification);
        nameLabel.setFont(getInterRegularFont());
        nameLabel.setBorderSize(juce::BorderSize<int>(0));
        nameLabel.setJustificationType(juce::Justification::centredLeft);
        nameLabel.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(nameLabel);
        
        indexLabel.setText(juce::String::formatted("%02d", index + 1), juce::dontSendNotification);
        indexLabel.setFont(getInterRegularFont().withPointHeight(9));
        indexLabel.setBorderSize(juce::BorderSize<int>(0));
        indexLabel.setJustificationType(juce::Justification::centredRight);
        indexLabel.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(indexLabel);
        
        if (isFavouritable)
        {
            faveButton.setGradientIndex(gradientIndex);
            faveButton.setClickingTogglesState(true);
            addAndMakeVisible(faveButton);
        }
    }
    
    void setSelected(bool v)
    {
        isSelected = v;
        indexLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(isSelected ? 1.0f : 0.1f));
        repaint();
    }
    
    void paint(juce::Graphics &g) override
    {
        if (isSelected)
        {
            juce::Path roundedRect;
            roundedRect.addRoundedRectangle(getLocalBounds(), 2.0f);
            g.reduceClipRegion(roundedRect);
            
            g.fillAll(darkGrey);
            
            auto gradient = juce::ColourGradient::horizontal(gradientColour[0], 0.0f,
                                                             gradientColour[1], c_gradientWidth);
            g.setGradientFill(gradient);
            g.fillRect(0, 0, c_gradientWidth, getHeight());
        }
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds();
        
        indexLabel.setBounds(bounds.removeFromRight(20).withTrimmedRight(5));
        
        if (faveButton.isVisible())
        {
            faveButton.setBounds(bounds.removeFromLeft(16).withTrimmedLeft(4));
            nameLabel.setBounds(bounds.withTrimmedLeft(7));
        }
        else
        {
            nameLabel.setBounds(bounds.withTrimmedLeft(16));
        }
    }
    
    juce::Label nameLabel, indexLabel;
    FavouriteButton faveButton;
    juce::Colour gradientColour[2];
    
protected:
    bool isSelected;
    
    static constexpr int c_gradientWidth = 3;
};

//==============================================================================
class PresetMenu::Model : public juce::ListBoxModel
{
public:
    Model(AtomicEngine& e)
    : engine(e)
    {
    }
    
    int getNumRows() override = 0;
    
    void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override {}
    
    juce::Component* refreshComponentForRow(int rowNumber, bool isRowSelected, juce::Component *existingComponentToUpdate) override
    {
        // TODO: deleting the old component every time is inefficient, but the logic is easier
        
        if (existingComponentToUpdate)
            delete existingComponentToUpdate;
        
        auto newComp = createComponent(rowNumber);
        
        if (newComp)
            newComp->setSelected(isRowSelected);
        
        return newComp;
    }
    
    void selectedRowsChanged(int lastRowSelected) override
    {
        if (onSelectionChanged)
            onSelectionChanged(lastRowSelected);
    }
    
    std::function<void(int)> onSelectionChanged = nullptr;
    
protected:
    AtomicEngine& engine;
    
    virtual ListRowComponent* createComponent(int rowNumber) = 0;
    
    virtual void updateComponent(ListRowComponent* component, int rowNumber, bool isSelected)
    {
        component->setSelected(isSelected);
    }
};

//==============================================================================
class PresetMenu::CategoriesModel : public PresetMenu::Model
{
public:
    CategoriesModel(AtomicEngine& e)
    : Model(e)
    {
    }
    
    int getNumRows() override
    {
        return engine.getNumPresetCategories();
    }
    
protected:
    ListRowComponent* createComponent(int rowNumber) override
    {
        if (rowNumber < 0 || rowNumber >= engine.getNumPresetCategories())
            return nullptr;
        
        const auto& category = engine.getPresetCategory(rowNumber);
        return new ListRowComponent(category.name, category.index, 0, false);
    }
};

//==============================================================================
class PresetMenu::PresetsModel : public PresetMenu::Model
{
public:
    PresetsModel(AtomicEngine& e)
    : Model(e)
    {
    }
    
    void setPresets(const juce::Array<const AtomicEngine::Preset*>& ps)
    {
        presets = ps;
    }
    
    int getNumRows() override
    {
        return presets.size();
    }
    
protected:
    ListRowComponent* createComponent(int rowNumber) override
    {
        if (rowNumber < 0 || rowNumber >= presets.size())
            return nullptr;
        
        const auto& preset = *presets.getUnchecked(rowNumber);
        auto comp = new ListRowComponent(preset.name, preset.index, 2, true);
        comp->faveButton.setToggleState(preset.isFavourite, juce::dontSendNotification);
        comp->faveButton.onClick = [this, &preset, comp] {
            engine.setFavouriteState(preset, comp->faveButton.getToggleState());
        };
        return comp;
    }
    
private:
    juce::Array<const AtomicEngine::Preset*> presets;
};

//==============================================================================
class PresetMenu::SnapshotsModel : public PresetMenu::Model
{
public:
    SnapshotsModel(AtomicEngine& e)
    : Model(e)
    {
    }
    
    void setSnapshots(const juce::Array<const AtomicEngine::Snapshot*>& ss)
    {
        snapshots = ss;
    }
    
    int getNumRows() override
    {
        return snapshots.size();
    }
    
protected:
    ListRowComponent* createComponent(int rowNumber) override
    {
        if (rowNumber < 0 || rowNumber >= snapshots.size())
            return nullptr;
        
        const auto& snapshot = *snapshots.getUnchecked(rowNumber);
        auto comp = new ListRowComponent(snapshot.name, snapshot.index, 4, true);
        comp->faveButton.setToggleState(snapshot.isFavourite, juce::dontSendNotification);
        comp->faveButton.onClick = [this, &snapshot, comp] {
            engine.setFavouriteState(snapshot, comp->faveButton.getToggleState());
        };
        return comp;
    }
    
private:
    juce::Array<const AtomicEngine::Snapshot*> snapshots;
};

//==============================================================================
PresetMenu::PresetMenu(HydraAudioProcessor& p)
: processor(p)
{
    setLookAndFeel(&laf);
    
    searchBox.setMultiLine(false);
    searchBox.setFont(laf.interRegularFont);
    searchBox.setJustification(juce::Justification::centred);
    searchBox.setTextToShowWhenEmpty("Search", juce::Colour(0xFF515151));
    searchBox.onTextChange = [this] {
        populatePresetList();
        populateSnapshotList();
    };
    addAndMakeVisible(searchBox);
    
    searchIcon = juce::Drawable::createFromImageData(BinaryData::SearchIcon_svg, BinaryData::SearchIcon_svgSize);
    jassert(searchIcon != nullptr);
    if (searchIcon)
    {
        searchBox.addAndMakeVisible(*searchIcon);
    }
    
    filterButton[0].setButtonText("All");
    filterButton[1].setButtonText("Suggested");
    filterButton[2].setButtonText("Favourites");

    filterButton[0].setToggleState(true, juce::dontSendNotification);
    
    for (auto& button : filterButton)
    {
        button.setClickingTogglesState(true);
        button.setRadioGroupId(1);

        button.onClick = [this] {
            populatePresetList();
            populateSnapshotList();
        };

        addAndMakeVisible(button);
    }
    
    listHeading[0].setText("Category", juce::dontSendNotification);
    listHeading[1].setText("Preset", juce::dontSendNotification);
    listHeading[2].setText("Snapshot", juce::dontSendNotification);
    
    for (int i=0; i<3; i++)
    {
        auto& label = listHeading[i];
        label.setFont(laf.interRegularFont);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(GradientLabel::leftColourId, c_gradientColours[i*2]);
        label.setColour(GradientLabel::rightColourId, c_gradientColours[i*2+1]);
        addAndMakeVisible(label);
    }
    
    for (auto& list : listBox)
    {
        addAndMakeVisible(list);
    }
    
    categoriesModel = std::make_unique<CategoriesModel>(*processor.engine);
    listBox[0].setRowHeight(38);
    listBox[0].setModel(categoriesModel.get());
    
    presetsModel = std::make_unique<PresetsModel>(*processor.engine);
    listBox[1].setRowHeight(21);
    listBox[1].setModel(presetsModel.get());
    
    snapshotsModel = std::make_unique<SnapshotsModel>(*processor.engine);
    listBox[2].setRowHeight(21);
    listBox[2].setModel(snapshotsModel.get());
    
    categoriesModel->onSelectionChanged = [this](int index)
    {
        if (index >= 0 && index < processor.engine->getNumPresetCategories())
        {
            const auto& category = processor.engine->getPresetCategory(index);
            processor.engine->setSelectedPresetCategory(&category);
            populatePresetList();
        }
    };
    
    presetsModel->onSelectionChanged = [this](int index)
    {
        if (index >= 0 && index < filteredPresets.size())
        {
            const auto* preset = filteredPresets[index];
            processor.engine->setSelectedPreset(preset);
            populateSnapshotList();
        }
    };
    
    snapshotsModel->onSelectionChanged = [this](int index)
    {
        if (index >= 0 && index < filteredSnapshots.size())
        {
            const auto* snapshot = filteredSnapshots[index];
            processor.engine->setSelectedSnapshot(snapshot);
        }
    };
    
    updateSelection();
    
    processor.engine->addListener(this);
}

PresetMenu::~PresetMenu()
{
    processor.engine->removeListener(this);
    setLookAndFeel(nullptr);
}

//==============================================================================
PresetMenu::FilterType PresetMenu::getCurrentFilterType()
{
    if (filterButton[1].getToggleState())
        return FilterType::Suggested;
    else if (filterButton[2].getToggleState())
        return FilterType::Favourites;
    else
        return FilterType::All;
}

bool PresetMenu::testFilter(const AtomicEngine::Preset *preset)
{
    switch (getCurrentFilterType())
    {
        case FilterType::All:
            // Do nothing
            break;
            
        case FilterType::Suggested:
            for (auto* tag : preset->tags)
            {
                if (!tag->isTagActive())
                    return false;
            }
            break;
            
        case FilterType::Favourites:
            if (!preset->isFavourite)
                return false;
            break;
    }
    
    if (searchBox.getText().isNotEmpty()
        && !preset->name.containsIgnoreCase(searchBox.getText()))
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool PresetMenu::testFilter(const AtomicEngine::Snapshot *snapshot)
{
    // TODO: filter
    return true;
}

void PresetMenu::populatePresetList()
{
    filteredPresets.clear();
    
    auto* currentPreset = processor.engine->getSelectedPreset();
    int selectionIndex = -1;
    if (currentPreset)
    {
        auto* category = currentPreset->category;
            
        for (auto* preset : category->presets)
        {
            if (testFilter(preset))
            {
                filteredPresets.add(preset);
                
                if (preset == currentPreset)
                    selectionIndex = filteredPresets.size() - 1;
            }
        }
        
        presetsModel->setPresets(filteredPresets);
    }
    else
    {
        presetsModel->setPresets({});
    }
    
    listBox[1].updateContent();
    listBox[1].selectRow(selectionIndex);
}

void PresetMenu::populateSnapshotList()
{
    filteredSnapshots.clear();
    
    auto* currentPreset = processor.engine->getSelectedPreset();
    int selectionIndex = -1;
    if (currentPreset && filteredPresets.contains(currentPreset))
    {
        auto* currentSnapshot = processor.engine->getSelectedSnapshot();

        for (auto* snapshot : currentPreset->snapshots)
        {
            if (testFilter(snapshot))
            {
                filteredSnapshots.add(snapshot);
                
                if (snapshot == currentSnapshot)
                    selectionIndex = filteredSnapshots.size() - 1;
            }
        }
        
        snapshotsModel->setSnapshots(filteredSnapshots);
    }
    else
    {
        snapshotsModel->setSnapshots({});
    }
    
    listBox[2].updateContent();
    listBox[2].selectRow(selectionIndex);
}

//==============================================================================
void PresetMenu::paint (juce::Graphics& g)
{
}

void PresetMenu::resized()
{
    auto bounds = getLocalBounds();
    
    auto searchBoxBounds = bounds.removeFromTop(26);
    searchBoxBounds = searchBoxBounds.withSizeKeepingCentre(300, searchBoxBounds.getHeight());
    searchBox.setBounds(searchBoxBounds);
    
    if (searchIcon)
    {
        auto searchIconBounds = juce::Rectangle<float>(0, 0, searchBoxBounds.getHeight(), searchBoxBounds.getHeight());
        searchIconBounds = searchIconBounds.withSizeKeepingCentre(12, 12);
        searchIcon->setTransformToFit(searchIconBounds, juce::RectanglePlacement::centred);
    }
    
    bounds.removeFromTop(17);
    
    using Fr = juce::Grid::Fr;
    using Px = juce::Grid::Px;
    using GridItem = juce::GridItem;
    
    juce::Grid grid;
    grid.templateColumns = { Fr(1), Fr(1), Fr(1) };
    grid.columnGap = Px(5);
    grid.templateRows = { Px(30), Px(25), Px(14), Fr(1) };
    grid.items = {
        GridItem(filterButton[0]), GridItem(filterButton[1]), GridItem(filterButton[2]),
        GridItem(listHeading[0]), GridItem(listHeading[1]), GridItem(listHeading[2]),
        GridItem(), GridItem(), GridItem(),
        GridItem(listBox[0]), GridItem(listBox[1]), GridItem(listBox[2]),
    };
    
    grid.performLayout(bounds);
}

void PresetMenu::updateSelection()
{
    jassert(!isUpdatingSelection);
    juce::ScopedValueSetter<bool> recursiveCallGuard(isUpdatingSelection, true);
    
    auto* selectedPreset = processor.engine->getSelectedPreset();
    
    if (selectedPreset)
    {
        int categoryIndex = processor.engine->findPresetCategoryIndex(selectedPreset->category);
        listBox[0].selectRow(categoryIndex);
        
        int presetIndex = filteredPresets.indexOf(selectedPreset);
        listBox[1].selectRow(presetIndex);
        
        auto* selectedSnapshot = processor.engine->getSelectedSnapshot();
        if (selectedSnapshot)
        {
            int snapshotIndex = filteredSnapshots.indexOf(selectedSnapshot);
            listBox[2].selectRow(snapshotIndex);
        }
        else
        {
            listBox[2].deselectAllRows();
        }
    }
    else
    {
        for (int i=0; i<3; i++)
            listBox[i].deselectAllRows();
    }
}

//==============================================================================
void PresetMenu::engineSelectedPresetChanged()
{
    updateSelection();
    populateSnapshotList();
}

void PresetMenu::engineSelectedSnapshotChanged()
{
    updateSelection();
}

void PresetMenu::engineSnapshotsChangedForPreset(const AtomicEngine::Preset &preset)
{
    if (&preset == processor.engine->getSelectedPreset())
        populateSnapshotList();
}

void PresetMenu::enginePresetFavouriteStateChanged(const AtomicEngine::Preset &preset)
{
    if (getCurrentFilterType() == FilterType::Favourites)
    {
        populatePresetList();
    }
}

void PresetMenu::engineSnapshotFavouriteStateChanged(const AtomicEngine::Snapshot &snapshot)
{
    if (getCurrentFilterType() == FilterType::Favourites)
    {
        populateSnapshotList();
    }
}

//==============================================================================
PresetMenuLookAndFeel::PresetMenuLookAndFeel()
: interRegularFont(getInterRegularFont())
{
    setColour(juce::TextEditor::backgroundColourId, darkGrey);
    setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    setColour(juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    
    setColour(juce::TextButton::buttonColourId, darkGrey);
    setColour(juce::TextButton::buttonOnColourId, juce::Colours::black);
    setColour(juce::TextButton::textColourOnId, grey2);
    setColour(juce::TextButton::textColourOffId, grey2);
    
    setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    
    setColour(juce::ScrollBar::thumbColourId, meterGrey);
    setColour(juce::ScrollBar::trackColourId, darkGrey);
}

juce::Font PresetMenuLookAndFeel::getTextButtonFont(juce::TextButton &, int buttonHeight)
{
    return interRegularFont;
}

void PresetMenuLookAndFeel::drawButtonBackground(juce::Graphics &g, juce::Button &button, const juce::Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::Colour colour = findColour(button.getToggleState()
                                     ? juce::TextButton::buttonOnColourId
                                     : juce::TextButton::buttonColourId);
    
    if (shouldDrawButtonAsHighlighted)
        colour = colour.brighter(0.1f);
    
    if (shouldDrawButtonAsDown)
        colour = colour.brighter(0.1f);
    
    g.fillAll(colour);
}

void PresetMenuLookAndFeel::drawScrollbar(juce::Graphics &g, juce::ScrollBar &scrollBar, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown)
{
    g.setColour(findColour(juce::ScrollBar::trackColourId));
    juce::Rectangle<float> bounds(x, y, width, height);
    g.fillRoundedRectangle(bounds.reduced(1.0f), 4.0f);
    
    juce::LookAndFeel_V4::drawScrollbar(g, scrollBar, x, y, width, height, isScrollbarVertical, thumbStartPosition, thumbSize, isMouseOver, isMouseDown);
}
