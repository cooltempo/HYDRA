/*
  ==============================================================================

    VisualiserProcessor.h
    Created: 9 Apr 2024 12:01:41am
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <glm/vec4.hpp>

#include "../atomicengine/bsfx/Processor.h"

//==============================================================================
class VisualiserProcessor : public bsfx::Processor
{
public:
    VisualiserProcessor();
    
    // This isn't actually a functioning processor -- it's just a holder for parameters
    void prepareToPlay(double sampleRate, int samplesPerBlock) override {}
    void reset() override {}
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override {}
    
    juce::AudioParameterFloat* paramCameraYPos;
    juce::AudioParameterFloat* paramCameraZPos;
    juce::AudioParameterFloat* paramCameraFOV;
    juce::AudioParameterFloat* paramRotationSpeed;
    juce::AudioParameterInt* paramNumParticles;
    juce::AudioParameterFloat* paramParticleSize;
    juce::AudioParameterFloat* paramParticleSizeRandomness;
    juce::AudioParameterFloat* paramParticleSmoothness;
    juce::AudioParameterFloat* paramBackgroundFade;
    juce::AudioParameterFloat* paramActivationThreshold;
    juce::AudioParameterFloat* paramActivationAttack;
    juce::AudioParameterFloat* paramActivationRelease;
    juce::AudioParameterFloat* paramBaseRadius;
    juce::AudioParameterFloat* paramMovementStyle;
    juce::AudioParameterFloat* paramSnappySpeed;
    juce::AudioParameterFloat* paramForceScale;
    juce::AudioParameterFloat* paramDampingFactor;
    juce::AudioParameterFloat* paramJitterAmount;
    juce::AudioParameterFloat* paramEscapeSpeedMultiplier;
    juce::AudioParameterFloat* paramEscapeDuration;
    juce::AudioParameterFloat* paramMouseEffectRadius;
    juce::AudioParameterFloat* paramMouseRepulsion;
    juce::AudioParameterFloat* paramMouseEffectDuration;
    juce::AudioParameterFloat* paramHorizontalBandSize;
    juce::AudioParameterFloat* paramVerticalBandSize;
    juce::AudioParameterFloat* paramBandSmoothness;
    
    struct ColourParameters
    {
        ColourParameters(VisualiserProcessor& proc,
                         const juce::String& namePrefix,
                         const juce::String& idPrefix,
                         const juce::Colour& defaultValue);
        
        juce::AudioParameterFloat* red;
        juce::AudioParameterFloat* green;
        juce::AudioParameterFloat* blue;
        juce::AudioParameterFloat* alpha;
        
        juce::Colour getColour() const;
        glm::vec4 getVec4() const;
    };
    
    enum Region { Core, Equator, Meridian, North, South, c_numRegions };
    
    std::unique_ptr<ColourParameters> regionColours[c_numRegions];
    juce::AudioParameterFloat* paramRegionRadiusMod[c_numRegions];
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VisualiserProcessor)
};
