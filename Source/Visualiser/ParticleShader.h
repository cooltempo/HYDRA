/*
  ==============================================================================

    ParticleShader.h
    Created: 9 Apr 2024 12:06:55am
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

class ParticleShader : public bsgl::ShaderProgram
{
public:
    ParticleShader(juce::OpenGLContext& context)
    : bsgl::ShaderProgram(context,
                          juce::String(BinaryData::Particle_vert_glsl,
                                       BinaryData::Particle_vert_glslSize),
                          juce::String(BinaryData::Particle_frag_glsl,
                                       BinaryData::Particle_frag_glslSize)
                          )
    {
        BSGL_GET_UNIFORM(uniform_modelViewMatrix);
        BSGL_GET_UNIFORM(uniform_projectionMatrix);
        BSGL_GET_UNIFORM(uniform_particleSmoothness);
        BSGL_GET_UNIFORM(uniform_backgroundFadeCoefficient);
    }
    
    std::unique_ptr<Uniform> uniform_modelViewMatrix;
    std::unique_ptr<Uniform> uniform_projectionMatrix;
    std::unique_ptr<Uniform> uniform_particleSmoothness;
    std::unique_ptr<Uniform> uniform_backgroundFadeCoefficient;
    
    //=========================================================================
    struct Vertex
    {
        glm::vec2 in_uv;
    };

    // All information about particles which is passed to the GPU
    struct Instance
    {
        glm::vec3 in_particlePosition;
        glm::vec4 in_colour;
        float in_particleSize;
    };

    //=========================================================================
    class Buffer : public bsgl::BufferObject<Vertex, Instance>
    {
    public:
        Buffer(juce::OpenGLShaderProgram& shader)
        : bsgl::BufferObject<Vertex, Instance>(shader)
        {
        }
        
        juce::Array<AttributeInfo> getVertexAttributes() override
        {
            return {
                BSGL_VERTEX_ATTRIBUTE_INFO(in_uv)
            };
        }
        
        juce::Array<AttributeInfo> getInstanceAttributes() override
        {
            return {
                BSGL_INSTANCE_ATTRIBUTE_INFO(in_particlePosition),
                BSGL_INSTANCE_ATTRIBUTE_INFO(in_colour),
                BSGL_INSTANCE_ATTRIBUTE_INFO(in_particleSize),
            };
        }
    };


};
