/*
  ==============================================================================

    Visualiser.h
    Created: 2 Apr 2024 3:06:40pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <glm/glm.hpp>

#include "../bsgl/FPSCounter.h"
#include "../bsgl/ShaderProgram.h"
#include "../bsgl/BufferObject.h"

#include "VisualiserProcessor.h"
#include "ParticleShader.h"
#include "ScreenQuadImageShader.h"

class HydraAudioProcessor;

//==============================================================================
class Visualiser  : public juce::Component, public juce::OpenGLRenderer, public juce::AudioProcessorParameter::Listener
{
public:
    Visualiser(HydraAudioProcessor& pluginProc);
    ~Visualiser() override;

    void resized() override;
    
    void mouseEnter(const juce::MouseEvent &event) override;
    void mouseExit(const juce::MouseEvent &event) override;
    void mouseMove(const juce::MouseEvent &event) override;
    void mouseDoubleClick(const juce::MouseEvent &event) override;
    
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;
    
    VisualiserProcessor& processor;
    
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}

    bsgl::FPSCounter fpsCounter;
    
    using Region = VisualiserProcessor::Region;

    //========================================================================
private:
    juce::CriticalSection criticalSection;
    HydraAudioProcessor& pluginProcessor;
    juce::OpenGLContext context;

    //========================================================================
    juce::Random rng;
    
    inline float randomFloat(float minValue, float maxValue)
    {
        return juce::jmap(rng.nextFloat(), minValue, maxValue);
    }

    //========================================================================
    class Error : public std::exception
    {
    public:
        Error(const juce::String& msg) : message(msg) {}
        const juce::String& getMessage() const noexcept { return message; }
        const char* what() const noexcept override { return message.toRawUTF8(); }
        
    private:
        juce::String message;
    };
    
    //========================================================================
    struct Particle
    {
        glm::vec3 initialPositionNorm;
        glm::vec3 velocity;
        float mouseSnappinessMultiplier;
        float escapeSnappinessMultiplier;
        float escapeProbability;
    };
    
    //========================================================================
    struct State
    {
        State(juce::OpenGLContext& context);
        ~State();
        
        std::unique_ptr<ScreenQuadImageShader> backgroundShader;
        std::unique_ptr<ScreenQuadImageShader::Buffer> backgroundBuffer;
        std::unique_ptr<juce::OpenGLTexture> backgroundTexture;

        std::unique_ptr<ParticleShader> particleShader;
        std::unique_ptr<ParticleShader::Buffer> particleBuffer;
    };
    
    std::unique_ptr<State> state;
    
    void recalculateBackgroundUVs();
    
    //========================================================================
    int64_t lastFrameTime = 0.0;
    float rotationAngle = 0.0f;
    float activationValue = 0.0f;
    
    //========================================================================
    std::atomic<glm::vec2> currentMousePosition { glm::vec2 { -2.0f, -2.0f } };
    glm::vec2 mousePositionLastFrame { 0.0f, 0.0f };
    
    //========================================================================
    juce::Array<ParticleShader::Instance> instances;
    juce::Array<Particle> particles;
    void initParticles(bool clearExisting);
    void updateParticles(float deltaTime);

    //========================================================================
    glm::vec3 cameraPos;
    glm::mat4 modelMatrix, invModelMatrix;
    glm::mat4 viewMatrix, invViewMatrix;
    glm::mat4 projMatrix, invProjMatrix;
    void updateViewProjMatrices();

    //========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualiser)
};
