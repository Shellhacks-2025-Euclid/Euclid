#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <span>
#include <array>

namespace Euclid
{
class FrameBuffer {
public:
    FrameBuffer(glm::vec2 size);
    
    void SetSize(glm::vec2 size);
    
    uint32_t GetFBO();
    glm::vec2 GetSize();
    uint32_t GetTextureID();
    
    ~FrameBuffer();
    
private:
    void GenTexture();
    void BindBuffer();
    void UnBindBuffer();
    
private:
    uint32_t mFBO;
    uint32_t mTextureID;
    glm::vec2 mSize;
};

class Shader {
public:
    void Init(GLenum shaderType, const char* shaderCode);
    unsigned int GetID();
    
    ~Shader();
    
private:
    void CheckCompileErrors();
    
private:
    unsigned int mShaderID;
};

class ShaderProgram {
public:
    void Init(std::span<unsigned int> shaderIDs);
    unsigned int GetID();
    void Use();
    
    ~ShaderProgram();
    
private:
    void CheckCompileErrors();
    
private:
    unsigned int mProgramID;
};
}


