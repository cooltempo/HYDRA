/*
  ==============================================================================

    MessageThreadUtils.h
    Created: 17 Apr 2023 2:09:13pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/// If called from the message thread, call f synchronously. Otherwise, call f asynchronously.
inline void callSyncOrAsync(std::function<void()> f)
{
    if (juce::MessageManager::existsAndIsCurrentThread())
    {
        f();
    }
    else
    {
        juce::MessageManager::callAsync(f);
    }
}
