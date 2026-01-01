/*
  ==============================================================================

    ShaderProgram.h
    Created: 8 Apr 2024 7:26:42pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "GLError.h"

namespace bsgl {

//=============================================================================
#define BSGL_GET_UNIFORM(NAME)  NAME = getUniform(#NAME);

//=============================================================================
class ShaderProgram
{
public:
    ShaderProgram(juce::OpenGLContext& context,
                  const juce::String& vertexShaderCode,
                  const juce::String& fragmentShaderCode)
    {
        // CHECK_RESULT(..., shader->getLastError())
        
        shader = std::make_unique<juce::OpenGLShaderProgram>(context);

        shader->addVertexShader(vertexShaderCode);
        shader->addFragmentShader(fragmentShaderCode);
        shader->link();

        BSGL_CHECK_ERROR;

        shader->use();
        
        BSGL_CHECK_ERROR;
    }
    
    virtual ~ShaderProgram() {}
    
    juce::OpenGLShaderProgram& getShader() { return *shader; }
    
    void use()
    {
        shader->use();
        BSGL_CHECK_ERROR;
    }
    
    using Uniform = juce::OpenGLShaderProgram::Uniform;
    
protected:
    std::unique_ptr<Uniform> getUniform(const char* name)
    {
        return std::make_unique<Uniform>(*shader, name);
    }
    
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
};


} // namespace bsgl

