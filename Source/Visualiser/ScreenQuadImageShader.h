/*
  ==============================================================================

    ScreenQuadImageShader.h
    Created: 9 Apr 2024 12:13:53am
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

class ScreenQuadImageShader : public bsgl::ShaderProgram
{
public:
    ScreenQuadImageShader(juce::OpenGLContext& context)
    : bsgl::ShaderProgram(context,
                          juce::String(BinaryData::ScreenQuadImage_vert_glsl,
                                       BinaryData::ScreenQuadImage_vert_glslSize),
                          juce::String(BinaryData::ScreenQuadImage_frag_glsl,
                                       BinaryData::ScreenQuadImage_frag_glslSize)
                          )
    {
        BSGL_GET_UNIFORM(uniform_texture);
    }
    
    std::unique_ptr<Uniform> uniform_texture;
    
    //=========================================================================
    struct Vertex
    {
        glm::vec2 in_position;
        glm::vec2 in_uv;
    };

    //=========================================================================
    class Buffer : public bsgl::BufferObject<Vertex>
    {
    public:
        Buffer(juce::OpenGLShaderProgram& shader)
        : bsgl::BufferObject<Vertex>(shader)
        {
        }
        
        juce::Array<AttributeInfo> getVertexAttributes() override
        {
            return {
                BSGL_VERTEX_ATTRIBUTE_INFO(in_position),
                BSGL_VERTEX_ATTRIBUTE_INFO(in_uv)
            };
        }
        
        juce::Array<AttributeInfo> getInstanceAttributes() override
        {
            return {};
        }
    };
};
