/*
  ==============================================================================

    VisualiserProcessor.cpp
    Created: 9 Apr 2024 12:01:41am
    Author:  Ed Powley

  ==============================================================================
*/

#include "VisualiserProcessor.h"

VisualiserProcessor::VisualiserProcessor()
{
    addParameter(paramCameraYPos = new juce::AudioParameterFloat
                 ({ "paramCameraYPos", 1 },
                  "Camera Y position",
                  juce::NormalisableRange<float>(-10.0f, 10.0f),
                  2.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToStringWithSign<2>)
                  ));
    
    addParameter(paramCameraZPos = new juce::AudioParameterFloat
                 ({ "paramCameraZPos", 1 },
                  "Camera Z position",
                  juce::NormalisableRange<float>(0.1f, 20.0f),
                  5.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<2>)
                  ));

    addParameter(paramCameraFOV = new juce::AudioParameterFloat
                 ({ "paramCameraFOV", 1 },
                  "Camera field of view",
                  juce::NormalisableRange<float>(10.0f, 170.0f),
                  90.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<2>)
                  .withLabel("degrees")
                  ));

    addParameter(paramRotationSpeed = new juce::AudioParameterFloat
                 ({ "paramRotationSpeed", 1 },
                  "Rotation speed",
                  juce::NormalisableRange<float>(-180.0f, 180.0f),
                  -10.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToStringWithSign<2>)
                  .withLabel("deg/sec")
                  ));
    
    addParameter(paramNumParticles = new juce::AudioParameterInt
                 ({ "paramNumParticles", 1 },
                  "Number of particles",
                  1, 10000,
                  2500));
    
    addParameter(paramParticleSize = new juce::AudioParameterFloat
                 ({ "paramParticleSize", 1 },
                  "Particle size",
                  juce::NormalisableRange<float>(0.0f, 0.25f),
                  0.05f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));
    
    addParameter(paramParticleSizeRandomness = new juce::AudioParameterFloat
                 ({ "paramParticleSizeRandomness", 1 },
                  "Particle size randomness",
                  juce::NormalisableRange<float>(0.0f, 1.0f),
                  0.25f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::percentToString<1>)
                  .withValueFromStringFunction(bsfx::stringToPercent)
                  .withLabel("%")
                  ));
    
    addParameter(paramParticleSmoothness = new juce::AudioParameterFloat
                 ({ "paramParticleSmoothness", 1 },
                  "Particle edge smoothness",
                  juce::NormalisableRange<float>(0.0001f, 1.0f),
                  0.1f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));
    
    addParameter(paramBackgroundFade = new juce::AudioParameterFloat
                 ({ "paramBackgroundFade", 1 },
                  "Background fade amount",
                  juce::NormalisableRange<float>(0.0f, 1.0f),
                  0.5f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::percentToString<1>)
                  .withValueFromStringFunction(bsfx::stringToPercent)
                  .withLabel("%")
                  ));
    
    addParameter(paramActivationThreshold = bsfx::newGainParameter
                 ({ "paramActivationThreshold", 1 },
                  "Activation threshold",
                  0.0f, 1.0f,
                  juce::Decibels::decibelsToGain(-48.0f)
                  ));
    
    addParameter(paramActivationAttack = new juce::AudioParameterFloat
                 ({ "paramActivationAttack", 1 },
                  "Activation attack",
                  juce::NormalisableRange<float>(0.0f, 2000.0f),
                  250.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<2>)
                  .withLabel("ms")
                  ));
    
    addParameter(paramActivationRelease = new juce::AudioParameterFloat
                 ({ "paramActivationRelease", 1 },
                  "Activation release",
                  juce::NormalisableRange<float>(0.0f, 2000.0f),
                  250.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<2>)
                  .withLabel("ms")
                  ));
    
    addParameter(paramBaseRadius = new juce::AudioParameterFloat
                 ({ "paramBaseRadius", 1 },
                  "Base sphere radius",
                  juce::NormalisableRange<float>(0.1f, 5.0f),
                  3.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<2>)
                  ));

    addParameter(paramMovementStyle = new juce::AudioParameterFloat
                 ({ "paramMovementStype", 1 },
                  "Movement style (0=springy, 1=snappy)",
                  juce::NormalisableRange<float>(0.0f, 1.0f),
                  0.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));

    addParameter(paramSnappySpeed = new juce::AudioParameterFloat
                 ({ "paramSnappySpeed", 1 },
                  "Snappy movement speed",
                  juce::NormalisableRange<float>(0.0f, 1000.0f),
                  100.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<1>)
                  .withLabel("ms")
                  ));

    addParameter(paramForceScale = new juce::AudioParameterFloat
                 ({ "paramForceScale", 1 },
                  "Force scale",
                  skewRange(0.0f, 5.0f, 50.0f),
                  1.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));

    addParameter(paramDampingFactor = new juce::AudioParameterFloat
                 ({ "paramDampingFactor", 1 },
                  "Damping factor",
                  juce::NormalisableRange<float>(0.0f, 1.0f),
                  0.5f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));
    
    addParameter(paramJitterAmount = new juce::AudioParameterFloat
                 ({ "paramJitterAmount", 1 },
                  "Jitter amount",
                  skewRange(0.0f, 5.0f, 50.0f),
                  1.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));

    addParameter(paramEscapeSpeedMultiplier = new juce::AudioParameterFloat
                 ({ "paramEscapeSpeedMultiplier", 1 },
                  "Escape speed multiplier",
                  skewRange(0.0f, 0.1f, 1.0f),
                  0.1f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));

    addParameter(paramEscapeDuration = new juce::AudioParameterFloat
                 ({ "paramEscapeDuration", 1 },
                  "Escape effect duration per particle",
                  juce::NormalisableRange<float>(0.01f, 5.0f),
                  1.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<3>)
                  .withLabel("s")
                  ));

    addParameter(paramMouseEffectRadius = new juce::AudioParameterFloat
                 ({ "paramMouseEffectRadius", 1 },
                  "Mouse effect radius",
                  juce::NormalisableRange<float>(0.0f, 2.0f),
                  1.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));
    
    addParameter(paramMouseRepulsion = new juce::AudioParameterFloat
                 ({ "paramMouseRepulsion", 1 },
                  "Mouse repulsion(+) / attraction(-) force",
                  juce::NormalisableRange<float>(-50.0f, 50.0f),
                  10.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));

    addParameter(paramMouseEffectDuration = new juce::AudioParameterFloat
                 ({ "paramMouseEffectDuration", 1 },
                  "Mouse effect duration per particle",
                  juce::NormalisableRange<float>(0.01f, 5.0f),
                  1.0f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<3>)
                  .withLabel("s")
                  ));

    addParameter(paramHorizontalBandSize = new juce::AudioParameterFloat
                 ({ "paramHorizontalBandSize", 1 },
                  "Horizontal band size",
                  juce::NormalisableRange<float>(0.0001f, 1.0f),
                  0.2f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::percentToString<1>)
                  .withValueFromStringFunction(bsfx::stringToPercent)
                  .withLabel("%")
                  ));

    addParameter(paramVerticalBandSize = new juce::AudioParameterFloat
                 ({ "paramVerticalBandSize", 1 },
                  "Vertical band size",
                  juce::NormalisableRange<float>(0.0001f, 1.0f),
                  0.2f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::percentToString<1>)
                  .withValueFromStringFunction(bsfx::stringToPercent)
                  .withLabel("%")
                  ));
    
    addParameter(paramBandSmoothness = new juce::AudioParameterFloat
                 ({ "paramBandSmoothness", 1 },
                  "Band edge smoothness",
                  juce::NormalisableRange<float>(0.0001f, 2.0f),
                  0.2f,
                  juce::AudioParameterFloatAttributes()
                  .withStringFromValueFunction(bsfx::decimalToString<4>)
                  ));
    
    juce::Colour defaultColours[c_numRegions] = {
        juce::Colours::cyan,
        juce::Colours::yellow,
        juce::Colours::blue,
        juce::Colours::green,
        juce::Colours::red
    };
    
    auto initRegionParams = [this, &defaultColours](Region region, const juce::String& regionName)
    {
        regionColours[region] = std::make_unique<ColourParameters>(*this, regionName, "regionColours[" + regionName + "]", defaultColours[region]);
        addParameter(paramRegionRadiusMod[region] = new juce::AudioParameterFloat
                     ({ "paramRegionRadiusMod[" + regionName + "]", 1 },
                      regionName + " radius modifier",
                      juce::NormalisableRange<float>(0.0f, 0.5f),
                      0.0f,
                      juce::AudioParameterFloatAttributes()
                      .withStringFromValueFunction(bsfx::percentToStringWithSign<1>)
                      .withValueFromStringFunction(bsfx::stringToPercent)
                      .withLabel("%")
                      ));
    };
    
    initRegionParams(Core, "Core");
    initRegionParams(Equator, "Equator");
    initRegionParams(Meridian, "Meridian");
    initRegionParams(North, "North");
    initRegionParams(South, "South");
}

VisualiserProcessor::ColourParameters::ColourParameters(VisualiserProcessor &proc,
                                                         const juce::String &namePrefix,
                                                         const juce::String &idPrefix,
                                                         const juce::Colour& defaultValue)
{
    auto attributes = (juce::AudioParameterFloatAttributes()
                       .withStringFromValueFunction(bsfx::decimalToString<3>)
                       );
    
    proc.addParameter(red = new juce::AudioParameterFloat
                      ({ idPrefix + ".red", 1 },
                       namePrefix + " Red",
                       juce::NormalisableRange<float>(0.0f, 1.0f),
                       defaultValue.getFloatRed(),
                       attributes
                       ));

    proc.addParameter(green = new juce::AudioParameterFloat
                      ({ idPrefix + ".green", 1 },
                       namePrefix + " Green",
                       juce::NormalisableRange<float>(0.0f, 1.0f),
                       defaultValue.getFloatGreen(),
                       attributes
                       ));

    proc.addParameter(blue = new juce::AudioParameterFloat
                      ({ idPrefix + ".blue", 1 },
                       namePrefix + " Blue",
                       juce::NormalisableRange<float>(0.0f, 1.0f),
                       defaultValue.getFloatBlue(),
                       attributes
                       ));

    proc.addParameter(alpha = new juce::AudioParameterFloat
                      ({ idPrefix + ".alpha", 1 },
                       namePrefix + " Alpha",
                       juce::NormalisableRange<float>(0.0f, 1.0f),
                       defaultValue.getFloatAlpha(),
                       attributes
                       ));
}

juce::Colour VisualiserProcessor::ColourParameters::getColour() const
{
    return juce::Colour::fromFloatRGBA(red->get(),
                                       green->get(),
                                       blue->get(),
                                       alpha->get());
}

glm::vec4 VisualiserProcessor::ColourParameters::getVec4() const
{
    return {
        red->get(),
        green->get(),
        blue->get(),
        alpha->get()
    };
}
