/*
  ==============================================================================

    InstancedBufferObject.h
    Created: 8 Apr 2024 7:42:34pm
    Author:  Ed Powley

  ==============================================================================
*/

#pragma once

#include "GLError.h"
#include "ShaderProgram.h"

namespace bsgl {

using namespace juce::gl;

struct NotInstanced {};

//=============================================================================
template<typename VertexType, typename InstanceType = NotInstanced>
class BufferObject
{
public:
    static constexpr bool c_isInstanced = !std::is_same_v<InstanceType, NotInstanced>;
    
    BufferObject(juce::OpenGLShaderProgram& shader)
    : m_shader(shader)
    {
    }
    
    virtual ~BufferObject()
    {
        glDeleteBuffers(1, &m_vertexBufferObject);
        glDeleteBuffers(1, &m_elementBufferObject);
        glDeleteBuffers(1, &m_instanceBufferObject);
        glDeleteVertexArrays(1, &m_vertexArrayObject);
    }
    
    void init()
    {
        glGenBuffers(1, &m_vertexBufferObject);
        glGenBuffers(1, &m_elementBufferObject);
        glGenVertexArrays(1, &m_vertexArrayObject);
        
        glBindVertexArray(m_vertexArrayObject);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
        
        for (const AttributeInfo& attrib : getVertexAttributes())
        {
            attrib.enable();
        }
        
        BSGL_CHECK_ERROR;
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
        
        if constexpr (c_isInstanced)
        {
            glGenBuffers(1, &m_instanceBufferObject);
            glBindBuffer(GL_ARRAY_BUFFER, m_instanceBufferObject);
            
            for (const AttributeInfo& attrib : getInstanceAttributes())
            {
                attrib.enable();
                glVertexAttribDivisor(attrib.m_attributeId, 1);
            }

            BSGL_CHECK_ERROR;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        glBindVertexArray(0);

        BSGL_CHECK_ERROR;
    }
    
    void bindVertexArray()
    {
        glBindVertexArray(m_vertexArrayObject);
        
        BSGL_CHECK_ERROR;
    }
    
private:
    template<typename ArrayType, typename ElementType>
    static void setBufferData(const ArrayType& data, GLenum target, GLenum usage)
    {
        const ElementType* begin = std::begin(data);
        const ElementType* end = std::end(data);
        intptr_t numBytes = reinterpret_cast<intptr_t>(end) - reinterpret_cast<intptr_t>(begin);
        
        glBufferData(target, numBytes, begin, usage);

        BSGL_CHECK_ERROR;
    }
    
public:
    template<typename ArrayType>
    void setVertexData(const ArrayType& data, GLenum usage)
    {
        glBindVertexArray(m_vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObject);
        setBufferData<ArrayType, VertexType>(data, GL_ARRAY_BUFFER, usage);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    template<typename ArrayType>
    void setInstanceData(const ArrayType& data, GLenum usage)
    {
        glBindVertexArray(m_vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, m_instanceBufferObject);
        setBufferData<ArrayType, InstanceType>(data, GL_ARRAY_BUFFER, usage);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    template<typename ArrayType>
    void setIndexData(const ArrayType& data, GLenum usage)
    {
        glBindVertexArray(m_vertexArrayObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
        setBufferData<ArrayType, unsigned int>(data, GL_ELEMENT_ARRAY_BUFFER, usage);
    }
    
    //=========================================================================
protected:
    juce::OpenGLShaderProgram& m_shader;

    template<typename T> static std::pair<GLint, GLenum> getAttributeSizeAndType();
    template<> static std::pair<GLint, GLenum> getAttributeSizeAndType<float>()     { return { 1, GL_FLOAT }; }
    template<> static std::pair<GLint, GLenum> getAttributeSizeAndType<glm::vec2>() { return { 2, GL_FLOAT }; }
    template<> static std::pair<GLint, GLenum> getAttributeSizeAndType<glm::vec3>() { return { 3, GL_FLOAT }; }
    template<> static std::pair<GLint, GLenum> getAttributeSizeAndType<glm::vec4>() { return { 4, GL_FLOAT }; }
    
    using TVertex = VertexType;
    using TInstance = InstanceType;

    struct AttributeInfo
    {
    public:
        AttributeInfo(juce::OpenGLShaderProgram& shader, const char* name)
        : m_name(name)
        {
            m_attributeId = glGetAttribLocation(shader.getProgramID(), m_name);
            jassert(m_attributeId >= 0);
        }
        
        AttributeInfo withStrideAndOffset(GLsizei stride, GLsizei offset) const
        {
            AttributeInfo result = *this;
            result.m_stride = stride;
            result.m_offset = offset;
            return result;
        }
        
        template<typename T>
        AttributeInfo withType() const
        {
            AttributeInfo result = *this;
            auto sizeAndType = getAttributeSizeAndType<T>();
            result.m_size = sizeAndType.first;
            result.m_type = sizeAndType.second;
            return result;
        }
        
        void enable() const
        {
            glVertexAttribPointer(m_attributeId,
                                  m_size,
                                  m_type,
                                  GL_FALSE,
                                  m_stride,
                                  reinterpret_cast<void*>(m_offset));
            glEnableVertexAttribArray(m_attributeId);

            BSGL_CHECK_ERROR;
        }
        
        GLint m_attributeId;
        const char* m_name;
        GLsizei m_stride = 0;
        GLsizei m_offset = 0;
        GLint m_size = 0;
        GLenum m_type = 0;
    };
    
    struct VertexAttributeInfo : public AttributeInfo {};
    struct InstanceAttributeInfo : public AttributeInfo {};
        
    //=========================================================================
#define BSGL_ATTRIBUTE_INFO(STRUCT, NAME) \
    AttributeInfo(m_shader, #NAME) \
    .withStrideAndOffset(sizeof(STRUCT), offsetof(STRUCT, NAME)) \
    .withType<decltype(STRUCT::NAME)>()
    
#define BSGL_VERTEX_ATTRIBUTE_INFO(NAME)  BSGL_ATTRIBUTE_INFO(TVertex, NAME)
#define BSGL_INSTANCE_ATTRIBUTE_INFO(NAME)  BSGL_ATTRIBUTE_INFO(TInstance, NAME)

    virtual juce::Array<AttributeInfo> getVertexAttributes() = 0;
    virtual juce::Array<AttributeInfo> getInstanceAttributes() = 0;
    
private:
    GLuint m_vertexBufferObject = 0;
    GLuint m_vertexArrayObject = 0;
    GLuint m_elementBufferObject = 0;
    GLuint m_instanceBufferObject = 0;
};

//=============================================================================
} // namespace bsgl

