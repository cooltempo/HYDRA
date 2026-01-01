/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HydraAudioProcessorEditor::HydraAudioProcessorEditor (HydraAudioProcessor& p)
: audioProcessor (p)
, saveButton("Save Button", juce::DrawableButton::ButtonStyle::ImageFitted)
, settingsButton("Settings Button", juce::DrawableButton::ButtonStyle::ImageFitted)
, presetSelector(p)
, visualiser(p)
, presetMenu(p)
{
    backgroundImage = juce::ImageCache::getFromMemory(BinaryData::Background_png,
                                                      BinaryData::Background_pngSize);
    
    macroMapButton.addAttachment(p.paramMacroMap);
    macroMapButton.setButtonText(SwitchButton::Title::MacroMap);
    addAndMakeVisible(macroMapButton);
    
    bypassButton.addAttachment(p.paramBypass);
    bypassButton.setButtonText(SwitchButton::Title::Bypass);
    addAndMakeVisible(bypassButton);
    
    atomicButton.addAttachment(p.paramAtomic);
    atomicButton.setButtonText(SwitchButton::Title::Atomic);
    addAndMakeVisible(atomicButton);
    
    freezeButton.addAttachment(p.paramFreeze);
    freezeButton.setButtonText(SwitchButton::Title::Freeze);
    addAndMakeVisible(freezeButton);
    
    for (int i = 0; i < audioProcessor.engine->c_numMacroParameters; i++)
    {
        MacroKnob* knob = new MacroKnob();
        knob->setDisplayName(audioProcessor.engine->getMacroDisplayName(i));
        knob->setLeft(i < 4);
        addAndMakeVisible(knob);
        
        macroKnobs.add(std::move(knob));
    }
    
    int macroKnobIndex = 0;
    p.engine->forEachMacroParameter([this, &macroKnobIndex] (MacroParameter& mp) {
        macroKnobs[macroKnobIndex]->addAttachment(mp.param);
        macroKnobIndex++;
    });
    
    inputKnob.setText(BottomKnob::Function::Input);
    inputKnob.addAttachment(p.paramInputGain);
    addAndMakeVisible(inputKnob);
    
    mixKnob.setText(BottomKnob::Function::Mix);
    mixKnob.addAttachment(p.paramMix);
    addAndMakeVisible(mixKnob);
    
    outputKnob.setText(BottomKnob::Function::Output);
    outputKnob.addAttachment(p.paramOutputGain);
    addAndMakeVisible(outputKnob);
    
    audioProcessor.engine->addListener(this);

    addAndMakeVisible(atomicLogo);
    
    hamburgerIcon.onClick = [this] {
        bool toggled = hamburgerIcon.getToggleState();
        inputMeter.setVisible(!toggled);
        outputMeter.setVisible(!toggled);
        visualiser.setVisible(!toggled);
        presetMenu.setVisible(toggled);
    };
    
    addAndMakeVisible(hamburgerIcon);
    
    addAndMakeVisible(presetSelector);
    
    saveButton.onClick = [this] () {
        saveNameDialog = std::make_unique<juce::AlertWindow>("Save Snapshot", "Enter a name", juce::MessageBoxIconType::QuestionIcon, &saveButton);
        const auto* currentSnapshot = audioProcessor.engine->getSelectedSnapshot();
        saveNameDialog->addTextEditor("name", currentSnapshot ? currentSnapshot->name : "");
        saveNameDialog->addButton("Save", 1);
        saveNameDialog->addButton("Cancel", 0);
        
        saveNameDialog->enterModalState(true, juce::ModalCallbackFunction::create([this](int result) {
            if (result)
            {
                auto name = saveNameDialog->getTextEditorContents("name");
                if (name.isNotEmpty() && name != "Default")
                {
                    audioProcessor.engine->saveSnapshot(name);
                }
            }
            
            saveNameDialog = nullptr;
        }));
    };
    saveButton.setImages(juce::Drawable::createFromImageData(BinaryData::SaveIcon_svg,
                                                             BinaryData::SaveIcon_svgSize).get());
    addAndMakeVisible(saveButton);
    
    settingsButton.onClick = [] () {
            DBG("Settings Clicked");
        };
    settingsButton.setImages(juce::Drawable::createFromImageData(BinaryData::SettingsIcon_svg,
                                                                 BinaryData::SettingsIcon_svgSize).get());
    addAndMakeVisible(settingsButton);
    
    inputMeter.setProcessor(audioProcessor.inputMeter);
    inputMeter.setTitle("INPUT");
    addAndMakeVisible(inputMeter);
    
    outputMeter.setProcessor(audioProcessor.outputMeter);
    outputMeter.setTitle("OUTPUT");
    addAndMakeVisible(outputMeter);
    
    addAndMakeVisible(visualiser);
    
    addChildComponent(presetMenu);
    
#if HYDRA_ENABLE_DEV_MODE
    visualiserFPS.setFont(10);
    visualiserFPS.setColour(juce::Label::textColourId, juce::Colours::yellow);
    visualiser.fpsCounter.onNewReading = [this](int fps, double cpu) {
        visualiserFPS.setText(juce::String::formatted("%d FPS %.1f%% CPU", fps, cpu * 100.0),
                              juce::dontSendNotification);
    };
    addAndMakeVisible(visualiserFPS);

    parameterTree.add(*audioProcessor.inputMeter, "inputMeter");
    parameterTree.add(*audioProcessor.outputMeter, "outputMeter");
    parameterTree.add(visualiser.processor, "visualiser");
    
    auto tabColour = findColour(juce::ResizableWindow::backgroundColourId);
    devPanel.reset(new juce::TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtTop));
    devPanel->addTab("Dev parameters", tabColour, &parameterTree, false);
    devPanelButton.onClick = [this] {
        if (devPanel->isVisible())
        {
            devPanel->setVisible(false);
            setSize(getWidth() - c_devPanelWidth, getHeight());
        }
        else
        {
            devPanel->setVisible(true);
            setSize(getWidth() + c_devPanelWidth, getHeight());
        }
    };
    addAndMakeVisible(devPanelButton);
    addChildComponent(devPanel.get());
#endif
    
    startTimer(30);
    
    setSize (980, 765);
}

HydraAudioProcessorEditor::~HydraAudioProcessorEditor()
{
    audioProcessor.engine->removeListener(this);
}

//==============================================================================
void HydraAudioProcessorEditor::paint (juce::Graphics& g)
{
#if HYDRA_ENABLE_DEV_MODE
    const auto localBounds = getLocalBounds().withTrimmedRight(devPanel->isVisible() ? c_devPanelWidth : 0);
#else
    const auto localBounds = getLocalBounds();
#endif

    g.drawImage(backgroundImage, localBounds.toFloat());
}

void HydraAudioProcessorEditor::resized()
{
#if HYDRA_ENABLE_DEV_MODE
    const auto localBounds = getLocalBounds().withTrimmedRight(devPanel->isVisible() ? c_devPanelWidth : 0);
#else
    const auto localBounds = getLocalBounds();
#endif
    
    juce::Rectangle<int> leftButtonBounds(128, 650, 122, 58);
    macroMapButton.setBounds(leftButtonBounds.removeFromTop(25));
    bypassButton.setBounds(leftButtonBounds.removeFromBottom(25));
    
    juce::Rectangle<int> rightButtonBounds(732, 650, 122, 58);
    atomicButton.setBounds(rightButtonBounds.removeFromTop(25));
    freezeButton.setBounds(rightButtonBounds.removeFromBottom(25));
    
    
    atomicLogo.setBounds(185, 29, 29, 29);
    hamburgerIcon.setBounds(233, 29, 29, 29);
    
    juce::Rectangle<int> presetBounds(271, 18, 438, 47);
    presetSelector.setBounds(presetBounds);
    
    saveButton.setBounds(730, 29, 29, 29);
    settingsButton.setBounds(772, 29, 29, 29);
    
    juce::Rectangle<int> leftKnobBounds(24, 131, 188, 404);
    juce::Rectangle<int> rightKnobBounds(860-94, 131, 188, 404);
    for (int i = 0; i < macroKnobs.size(); i++)
    {
        juce::Rectangle<int>* currentBounds;
        
        if (i < 4)
            currentBounds = &leftKnobBounds;
        else
            currentBounds = &rightKnobBounds;
        
        macroKnobs[i]->setBounds(currentBounds->removeFromTop(90));
       
        if (i % 4 < 3)
            currentBounds->removeFromTop(15);
    }
    
    juce::Rectangle<int> bottomKnobBounds(307, 583, 388, 151);
    inputKnob.setBounds(bottomKnobBounds.removeFromLeft(94));
    bottomKnobBounds.removeFromLeft(53);
    mixKnob.setBounds(bottomKnobBounds.removeFromLeft(94));
    bottomKnobBounds.removeFromLeft(53);
    outputKnob.setBounds(bottomKnobBounds.removeFromLeft(94));
    
    juce::Rectangle<int> inputMeterBounds(126+98, 120+34, 47, 360);
    inputMeter.setBounds(inputMeterBounds);
    
    juce::Rectangle<int> outputMeterBounds(126+583, 120+34, 47, 360);
    outputMeter.setBounds(outputMeterBounds);
    
    juce::Rectangle<int> visualiserBounds(126+98+50, 120+34, 583-98-53, 360);
    visualiser.setBounds(visualiserBounds);
    
    juce::Rectangle<int> presetMenuBounds = visualiserBounds.getUnion(inputMeterBounds).getUnion(outputMeterBounds);
    presetMenu.setBounds(presetMenuBounds);
    
#if HYDRA_ENABLE_DEV_MODE
    devPanelButton.setBounds(juce::Rectangle<int>(localBounds).removeFromRight(50).removeFromTop(30));
    
    if (devPanel->isVisible())
    {
        devPanel->setBounds(getLocalBounds().removeFromRight(c_devPanelWidth));
    }

    visualiserFPS.setBounds(visualiserBounds.translated(0, visualiserBounds.getHeight()).withHeight(16));
#endif
}

void HydraAudioProcessorEditor::timerCallback()
{
    float inputLevel = audioProcessor.inputMeter->getMeterValue(0);
    
    if (inputLevel >= 0.3f)
        atomicLogo.setState(AnimatedIcon::State::Advance);
    else
        atomicLogo.setState(AnimatedIcon::State::Retreat);
    
    if (atomicLogo.getState() == AnimatedIcon::State::Alive)
        hamburgerIcon.setState(AnimatedIcon::State::Advance);
    else if  (atomicLogo.getState() == AnimatedIcon::State::Dead)
        hamburgerIcon.setState(AnimatedIcon::State::Retreat);
}

//==============================================================================
void HydraAudioProcessorEditor::engineMacroDisplayNameChanged(int macroIndex, juce::String name)
{
    macroKnobs[macroIndex]->setDisplayName(name);
}
