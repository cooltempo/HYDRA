/*
  ==============================================================================

    Visualiser.cpp
    Created: 2 Apr 2024 3:06:40pm
    Author:  Ed Powley

  ==============================================================================
*/

#include <JuceHeader.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Visualiser.h"
#include "../PluginProcessor.h"

using namespace juce::gl;

//==============================================================================
Visualiser::Visualiser(HydraAudioProcessor& pluginProc)
: processor(pluginProc.visualiserProcessor)
, pluginProcessor(pluginProc)
{
    setOpaque(true);
    context.setComponentPaintingEnabled(false);
    context.setOpenGLVersionRequired(juce::OpenGLContext::OpenGLVersion::openGL3_2);
    context.setRenderer(this);
    context.attachTo(*this);
    //context.setSwapInterval(0);
    context.setContinuousRepainting(true);
    
    for (auto* param : processor.getParameters())
    {
        param->addListener(this);
    }
    
    initParticles(true);
}

Visualiser::~Visualiser()
{
    context.detach();
    for (auto param : processor.getParameters())
    {
        param->removeListener(this);
    }
}

void Visualiser::resized()
{
    updateViewProjMatrices();
}

void Visualiser::mouseDoubleClick(const juce::MouseEvent &event)
{
    juce::ScopedLock csLock(criticalSection);
    
    initParticles(true);
}

//==============================================================================
void Visualiser::newOpenGLContextCreated()
{
    juce::ScopedLock csLock(criticalSection);
    
    DBG("newOpenGLContextCreated");
    
    state = std::make_unique<State>(context);
    
    recalculateBackgroundUVs();
}

void Visualiser::openGLContextClosing()
{
    juce::ScopedLock csLock(criticalSection);
    
    DBG("openGLContextClosing");
    
    state = nullptr;
}

//==============================================================================
void Visualiser::mouseEnter(const juce::MouseEvent &event)
{
    mouseMove(event);
    mousePositionLastFrame = currentMousePosition;
}

void Visualiser::mouseExit(const juce::MouseEvent &event)
{
    currentMousePosition = glm::vec2 { -2.0f, -2.0f };
}

void Visualiser::mouseMove(const juce::MouseEvent &event)
{
    currentMousePosition = glm::vec2 {
        juce::jmap<float>(event.getPosition().x, 0, getWidth(),  -1.0f, +1.0f),
        juce::jmap<float>(event.getPosition().y, 0, getHeight(), +1.0f, -1.0f)
    };
}


//==============================================================================
Visualiser::State::State(juce::OpenGLContext &context)
{
    particleShader = std::make_unique<ParticleShader>(context);
    
    particleBuffer = std::make_unique<ParticleShader::Buffer>(particleShader->getShader());
    particleBuffer->init();
    
    // For particles, vertex buffer is a single quad -- this is instanced for multiple particles
    
    ParticleShader::Vertex particleVertices[] = {
        { {  1.0f, -1.0f } },  // top right
        { {  1.0f,  1.0f } },  // bottom right
        { { -1.0f,  1.0f } },  // bottom left
        { { -1.0f, -1.0f } },  // top left
    };
    
    particleBuffer->setVertexData(particleVertices, GL_STATIC_DRAW);
        
    unsigned int quadIndices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    particleBuffer->setIndexData(quadIndices, GL_STATIC_DRAW);
    
    glBindVertexArray(0);
    
    backgroundShader = std::make_unique<ScreenQuadImageShader>(context);
    
    backgroundBuffer = std::make_unique<ScreenQuadImageShader::Buffer>(backgroundShader->getShader());
    backgroundBuffer->init();
    
    backgroundBuffer->setIndexData(quadIndices, GL_STATIC_DRAW);
    
    backgroundTexture = std::make_unique<juce::OpenGLTexture>();
    backgroundTexture->loadImage(juce::ImageCache::getFromMemory(BinaryData::Background_png, BinaryData::Background_pngSize));
}

//==============================================================================
Visualiser::State::~State()
{
}

//==============================================================================
void Visualiser::recalculateBackgroundUVs()
{
    if (!state)
        return;
    
    auto myBounds = getBoundsInParent().toFloat();
    auto parentBounds = getParentComponent()->getLocalBounds().toFloat();
    
    float uLeft   = myBounds.getX()      / parentBounds.getWidth();
    float uRight  = myBounds.getRight()  / parentBounds.getWidth();
    float vTop    = myBounds.getY()      / parentBounds.getHeight();
    float vBottom = myBounds.getBottom() / parentBounds.getHeight();
    
    ScreenQuadImageShader::Vertex backgroundVertices[] = {
        { {  1.0f,  1.0f }, { uRight, 1.0f - vTop    } },  // top right
        { {  1.0f, -1.0f }, { uRight, 1.0f - vBottom } },  // bottom right
        { { -1.0f, -1.0f }, { uLeft,  1.0f - vBottom } },  // bottom left
        { { -1.0f,  1.0f }, { uLeft,  1.0f - vTop    } },  // top left
    };
    
    state->backgroundBuffer->setVertexData(backgroundVertices, GL_STATIC_DRAW);
}

//==============================================================================
void Visualiser::initParticles(bool clearExisting)
{
    juce::ScopedLock csLock(criticalSection);
    
    if (clearExisting)
    {
        instances.clear();
        particles.clear();
    }
    
    int numInstances = processor.paramNumParticles->get();
    
    if (numInstances < instances.size())
    {
        instances.resize(numInstances);
        particles.resize(numInstances);
    }
    else
    {
        const float baseRadius = processor.paramBaseRadius->get();
        const float baseSize = processor.paramParticleSize->get();
        const float sizeRange = processor.paramParticleSizeRandomness->get();
        
        while (instances.size() < numInstances)
        {
            ParticleShader::Instance newInstance;
            
            // Generate point inside unit sphere, by rejection sampling
            glm::vec3 pos;
            do {
                pos = {
                    (rng.nextFloat() - 0.5f) * 2.0f,
                    (rng.nextFloat() - 0.5f) * 2.0f,
                    (rng.nextFloat() - 0.5f) * 2.0f
                };
            } while (glm::dot(pos, pos) > 1.0f);
            
            // Normalise onto the sphere
            pos = glm::normalize(pos);
            
            newInstance.in_particlePosition = pos * baseRadius;
            newInstance.in_colour = {
                rng.nextFloat(),
                rng.nextFloat(),
                rng.nextFloat(),
                1.0f
            };
            newInstance.in_particleSize = baseSize * randomFloat(1.0f - sizeRange, 1.0f + sizeRange);
            
            instances.add(newInstance);
            
            Particle newParticle;
            newParticle.initialPositionNorm = pos;
            newParticle.velocity = glm::vec3(0.0f);
            newParticle.mouseSnappinessMultiplier = 1.0f;
            newParticle.escapeSnappinessMultiplier = 1.0f;
            newParticle.escapeProbability = randomFloat(0.0f, 1.0f);

            particles.add(newParticle);
        }
    }
    
    jassert(instances.size() == particles.size());
}

//==============================================================================
void Visualiser::parameterValueChanged(int parameterIndex, float newValue)
{
    if (parameterIndex == processor.paramCameraYPos->getParameterIndex() ||
        parameterIndex == processor.paramCameraZPos->getParameterIndex() ||
        parameterIndex == processor.paramCameraFOV->getParameterIndex())
    {
        updateViewProjMatrices();
    }
    else if (parameterIndex == processor.paramNumParticles->getParameterIndex())
    {
        initParticles(false);
    }
    else if (parameterIndex == processor.paramParticleSize->getParameterIndex() ||
             parameterIndex == processor.paramParticleSizeRandomness->getParameterIndex())
    {
        initParticles(true);
    }
}

void Visualiser::updateViewProjMatrices()
{
    juce::ScopedLock csLock(criticalSection);
    
    if (getWidth() > 0 && getHeight() > 0)
    {
        projMatrix = glm::perspective(glm::radians(processor.paramCameraFOV->get()),
                                      getLocalBounds().toFloat().getAspectRatio(),
                                      0.1f,
                                      50.0f);
        invProjMatrix = glm::inverse(projMatrix);
        cameraPos = glm::vec3 { 0.0f, processor.paramCameraYPos->get(), processor.paramCameraZPos->get() };
        viewMatrix = glm::lookAt(cameraPos,
                                 glm::vec3 { 0.0f, 0.0f, 0.0f },
                                 glm::vec3 { 0.0f, 1.0f, 0.0f });
        invViewMatrix = glm::inverse(viewMatrix);
    }
}

//==============================================================================
template<typename T>
T bilinearInterpolate(const T& value00,
                      const T& value10,
                      const T& value01,
                      const T& value11,
                      const glm::vec2& p)
{
    T valueX0 = glm::mix(value00, value10, p.x);
    T valueX1 = glm::mix(value01, value11, p.x);
    return glm::mix(valueX0, valueX1, p.y);
}

template<typename T>
T calculateExponentialFactor(T timeMS, double sampleRate)
{
    static constexpr T log2 = static_cast<T>(0.6931471805599453);
    
    if (timeMS > 0.001)
        return static_cast<T>(exp(-log2 / (timeMS / 1000.0 * sampleRate)));
    else
        return static_cast<T>(0.0);
}

template<typename T, typename U>
T calculateExponentialMovingAverage(const T& lastValue, const T& inputValue, U expFactor)
{
    return inputValue + expFactor * (lastValue - inputValue);
}

//=============================================================================
void Visualiser::updateParticles(float deltaTime)
{
    const float inputVolume = pluginProcessor.inputMeter->getMeterValue(0);
    const float inputVolumeThreshold = processor.paramActivationThreshold->get();
    if (inputVolume >= inputVolumeThreshold || inputVolumeThreshold <= juce::Decibels::decibelsToGain(-99.0f))
    {
        if (activationValue < 1.0f)
        {
            activationValue += 1000.0f / processor.paramActivationAttack->get() * deltaTime;
            activationValue = juce::jlimit(0.0f, 1.0f, activationValue);
        }
    }
    else
    {
        if (activationValue > 0.0f)
        {
            activationValue -= 1000.0f / processor.paramActivationRelease->get() * deltaTime;
            activationValue = juce::jlimit(0.0f, 1.0f, activationValue);
        }
    }
    
    const float mouseEffectRadius = processor.paramMouseEffectRadius->get();
    const float mouseRepulsionForce = processor.paramMouseRepulsion->get();
    const float mouseSnapMultiplierStep = deltaTime / processor.paramMouseEffectDuration->get();
    const glm::vec2 mouseScreenPos = currentMousePosition;
    bool useMouse = (mouseScreenPos.x >= -1.0f && mouseEffectRadius > 0.0f);
    const glm::vec3 cameraWorldPos = invModelMatrix * glm::vec4 { cameraPos, 1.0f };
    
    glm::vec3 mouseRay;
    float mouseSpeed;
    if (useMouse)
    {
        glm::vec4 mouseWorldPos = invModelMatrix * invViewMatrix * invProjMatrix * glm::vec4(mouseScreenPos, 0.5f, 1.0f);
        mouseWorldPos /= mouseWorldPos.w;
        mouseRay = glm::vec3 { mouseWorldPos } - cameraWorldPos;
        mouseRay = glm::normalize(mouseRay);
        
        mouseSpeed = glm::distance(mousePositionLastFrame, mouseScreenPos) / deltaTime;
        if (juce::approximatelyEqual(mouseSpeed, 0.0f))
            useMouse = false;
        
        mousePositionLastFrame = mouseScreenPos;
    }

    const float escapeSpeedMultiplier = processor.paramEscapeSpeedMultiplier->get();
    const float escapeSnapMultiplierStep = deltaTime / 1.0f; processor.paramEscapeDuration->get();

    const float snapAmount = processor.paramMovementStyle->get();
    const float snapFactor = calculateExponentialFactor(processor.paramSnappySpeed->get(), 1.0 / deltaTime);
    
    const float velDampingFactor = pow(1.0f - processor.paramDampingFactor->get(), deltaTime);
    const float accelerationFactor = processor.paramForceScale->get();
    const float jitterFactor = processor.paramJitterAmount->get();
    const float baseRadius = processor.paramBaseRadius->get();
    const glm::vec2 bandSize(processor.paramHorizontalBandSize->get(),
                             processor.paramVerticalBandSize->get());
    const float bandSmoothness = processor.paramBandSmoothness->get();
    
    glm::vec4 regionColours[Region::c_numRegions];
    float regionTargetRadii[Region::c_numRegions];
    
    for (int i=0; i<Region::c_numRegions; i++)
    {
        regionColours[i] = glm::mix(glm::vec4(1.0f), processor.regionColours[i]->getVec4(), activationValue);
        regionTargetRadii[i] = baseRadius * glm::mix(1.0f, 1.0f + processor.paramRegionRadiusMod[i]->get(), activationValue);
    }

    glm::mat4 mvpMatrix = projMatrix * viewMatrix * modelMatrix;
    
    for (int i=0; i<instances.size(); i++)
    {
        auto& instance = instances.getReference(i);
        auto& particle = particles.getReference(i);
        
        glm::vec4 screenSpacePos4 = mvpMatrix * glm::vec4(instance.in_particlePosition, 1.0f);
        glm::vec2 screenSpacePos(screenSpacePos4.x / screenSpacePos4.w,
                                 screenSpacePos4.y / screenSpacePos4.w);
        
        // Without smoothing, bandCoord would have x = {0, +1} and y = {0, +1}
        // where 0 means the point is inside the central (equator or meridian) band
        // and 1 means it's outside
        glm::vec2 bandCoord = glm::smoothstep(bandSize * (1.0f - bandSmoothness),
                                              bandSize * (1.0f + bandSmoothness),
                                              glm::abs(screenSpacePos)
                                              );
        
        const Region region00 = Region::Core;
        const Region region10 = Region::Equator;
        const Region region01 = Region::Meridian;
        const Region region11 = (screenSpacePos.y > 0) ? Region::North : Region::South;

        instance.in_colour = bilinearInterpolate(regionColours[region00],
                                                 regionColours[region10],
                                                 regionColours[region01],
                                                 regionColours[region11],
                                                 bandCoord);

        float targetDistanceFromCentre = bilinearInterpolate(regionTargetRadii[region00],
                                                             regionTargetRadii[region10],
                                                             regionTargetRadii[region01],
                                                             regionTargetRadii[region11],
                                                             bandCoord);

        float currentDistanceFromCentre = glm::length(instance.in_particlePosition);
        float distanceDiff = currentDistanceFromCentre - targetDistanceFromCentre;
        auto normPos = instance.in_particlePosition / currentDistanceFromCentre;
        auto acceleration = -accelerationFactor * distanceDiff * normPos;
        
        if (distanceDiff > 0.0f)
        {
            auto jitter = distanceDiff * jitterFactor * glm::vec3 {
                rng.nextFloat() - 0.5f,
                rng.nextFloat() - 0.5f,
                rng.nextFloat() - 0.5f
            };
            
            // This causes the jitter vector to be tangent to the sphere's surface
            // by subtracting the projection towards the centre
            jitter -= glm::dot(jitter, normPos) * normPos;
            
            acceleration += jitter;
        }
        
        // Mouse response
        if (useMouse)
        {
            glm::vec3 positionRelativeToCamera = instance.in_particlePosition - cameraWorldPos;
            glm::vec3 projectionOntoMouseRay = glm::dot(positionRelativeToCamera, mouseRay) * mouseRay;
            glm::vec3 offsetFromMouseRay = positionRelativeToCamera - projectionOntoMouseRay;
            
            float distanceFromMouseRay = glm::length(offsetFromMouseRay);
            if (distanceFromMouseRay > 0.0f && distanceFromMouseRay < mouseEffectRadius)
            {
                float magnitude = 1.0f - distanceFromMouseRay / mouseEffectRadius;
                glm::vec3 mouseForce = magnitude * mouseRepulsionForce * offsetFromMouseRay / distanceFromMouseRay;
                acceleration += mouseForce * mouseSpeed;
                particle.mouseSnappinessMultiplier = 0.0f;
            }
        }

        // Snappiness
        auto snapPosition = calculateExponentialMovingAverage(instance.in_particlePosition,
                                                              targetDistanceFromCentre * particle.initialPositionNorm,
                                                              snapFactor);
        auto snapVelocity = (snapPosition - instance.in_particlePosition) / deltaTime;
        particle.velocity = glm::mix(particle.velocity,
                                     snapVelocity,
                                     snapAmount * particle.mouseSnappinessMultiplier * particle.escapeSnappinessMultiplier);

        // s = u t + 1/2 a t^2
        instance.in_particlePosition += particle.velocity * deltaTime + acceleration * (0.5f * deltaTime * deltaTime);
        
        // v = u + a t
        particle.velocity += acceleration * deltaTime;
        
        // Damping
        particle.velocity *= velDampingFactor;
        
        // Snapping to sphere when no activation
        instance.in_particlePosition = glm::mix(instance.in_particlePosition,
                                                particle.initialPositionNorm * baseRadius,
                                                (1.0f - activationValue) * particle.mouseSnappinessMultiplier);
        
        // Escape
        float radialSpeed = glm::dot(particle.velocity, normPos);
        if (radialSpeed * escapeSpeedMultiplier > particle.escapeProbability)
        {
            particle.escapeSnappinessMultiplier = 0.0f;
        }
        
        // Mouse effect decay
        if (particle.mouseSnappinessMultiplier < 1.0f)
        {
            particle.mouseSnappinessMultiplier = glm::min(particle.mouseSnappinessMultiplier + mouseSnapMultiplierStep, 1.0f);
        }

        if (particle.escapeSnappinessMultiplier < 1.0f)
        {
            particle.escapeSnappinessMultiplier = glm::min(particle.escapeSnappinessMultiplier + escapeSnapMultiplierStep, 1.0f);
        }
    }
}

//==============================================================================
void Visualiser::renderOpenGL()
{
    fpsCounter.beginFrame();

    // Calculate delta time
    int64_t frameTime = juce::Time::getHighResolutionTicks();
    float deltaTime = (float)juce::Time::highResolutionTicksToSeconds(frameTime - lastFrameTime);
    if (!isfinite(deltaTime) || deltaTime > 1.0)
        deltaTime = 1.0;
    lastFrameTime = frameTime;

    // Preparations
    juce::ScopedLock csLock(criticalSection);
    jassert (juce::OpenGLHelpers::isContextActive());
   
    // Update parameter routings
    // Note that this doesn't take the engine's atomic mode setting into account -- may need to change this?
    processor.processParameterRoutings(true);
    
    // Update rotation
    float rotationStep = glm::radians(processor.paramRotationSpeed->get()) * (float)deltaTime;
    rotationAngle = fmod(rotationAngle + rotationStep, juce::MathConstants<float>::twoPi);
    modelMatrix = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3 { 0.0f, 1.0f, 0.0f });
    invModelMatrix = glm::rotate(glm::mat4(1.0f), -rotationAngle, glm::vec3 { 0.0f, 1.0f, 0.0f });
    
    glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
    
    // Update particle positions etc
    updateParticles(deltaTime);

    // Clear and prepare for drawing
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set viewport
    auto desktopScale = (float)context.getRenderingScale();
    glViewport(0, 0,
               juce::roundToInt(desktopScale * (float) getWidth()),
               juce::roundToInt(desktopScale * (float) getHeight()));

    // Draw the background
    state->backgroundShader->use();
    state->backgroundBuffer->bindVertexArray();
    state->backgroundTexture->bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    state->backgroundTexture->unbind();
    glBindVertexArray(0);
    
    // Calculate fade factor
    float fadeCoefficient = processor.paramBackgroundFade->get() / processor.paramBaseRadius->get();

    // Activate the shader and pass uniforms in
    state->particleShader->use();
    state->particleShader->uniform_projectionMatrix->setMatrix4(glm::value_ptr(projMatrix), 1, false);
    state->particleShader->uniform_modelViewMatrix->setMatrix4(glm::value_ptr(modelViewMatrix), 1, false);
    state->particleShader->uniform_particleSmoothness->set(processor.paramParticleSmoothness->get());
    state->particleShader->uniform_backgroundFadeCoefficient->set(fadeCoefficient);
    
    state->particleBuffer->bindVertexArray();
    
    // Pass in updated instance data
    state->particleBuffer->setInstanceData(instances, GL_DYNAMIC_DRAW);

    // Draw the VAO
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, instances.size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    fpsCounter.endFrame();
}
