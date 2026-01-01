/*
  ==============================================================================

    GraphicsGlobals.h
    Created: 17 Mar 2024 1:20:49am
    Author:  Arthur Wilson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

inline const juce::Font getSubwayTickerFont()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::SubwayTickerGrid_ttf, BinaryData::SubwayTickerGrid_ttfSize);
    return juce::Font(typeface).withPointHeight(9.0f);
}

inline const juce::Font getInterSemiboldFont()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::InterSemiBold_ttf, BinaryData::InterSemiBold_ttfSize);
    return juce::Font(typeface).withPointHeight(11.0f);
}

inline const juce::Font getInterRegularFont()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::InterRegular_ttf, BinaryData::InterRegular_ttfSize);
    return juce::Font(typeface).withPointHeight(11.0f);
}

inline const juce::Font getInterMediumFont()
{
    static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::InterMedium_ttf, BinaryData::InterMedium_ttfSize);
    return juce::Font(typeface).withPointHeight(11.0f);
}

static const juce::Colour darkGrey = juce::Colour(0xFF151A1B);
static const juce::Colour meterGrey = juce::Colour(0xFF383838);
static const juce::Colour grey2 = juce::Colour(0xFFA1A6B2);
static const juce::Colour green = juce::Colour(0xFF12F20D);
static const juce::Colour red = juce::Colour(0xFFFF4E4E);
static const juce::Colour orange = juce::Colour(0xFFDECAC3);
static const juce::Colour knobLEDBlack = juce::Colour(0xFF0D0E10);


