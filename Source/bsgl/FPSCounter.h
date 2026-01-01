/*
  ==============================================================================

    FPSCounter.h
    Created: 8 Apr 2024 7:23:28pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

namespace bsgl {

class FPSCounter : public juce::Timer
{
public:
    FPSCounter()
    {
        startTimer(1000);
    }
    
    void timerCallback() override
    {
        currentFPS = framesThisSecond.load();
        currentCPU = juce::Time::highResolutionTicksToSeconds(ticksInFrameThisSecond);
        framesThisSecond = 0;
        ticksInFrameThisSecond = 0;
        
        if (onNewReading)
            onNewReading(currentFPS, currentCPU);
    }
    
    void beginFrame()
    {
        frameStartTime = juce::Time::getHighResolutionTicks();
    }
    
    void endFrame()
    {
        ticksInFrameThisSecond += juce::Time::getHighResolutionTicks() - frameStartTime;
        framesThisSecond++;
    }
    
    int getFPS() { return currentFPS; }
    
    std::function<void(int frames, double cpuTime)> onNewReading = nullptr;
    
private:
    std::atomic<int> currentFPS = 0;
    std::atomic<double> currentCPU = 0.0;
    std::atomic<int> framesThisSecond = 0;
    std::atomic<int64_t> frameStartTime = 0;
    std::atomic<int64_t> ticksInFrameThisSecond = 0;
};

} // namespace bsgl

