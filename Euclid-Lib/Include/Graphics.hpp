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

class Camera {
public:
    Camera(glm::vec3 target = {0.0f, 0.0f, 0.0f}, float radius = 5.0f);
    ~Camera();

    // --- Setters / Getters ---
    void SetTarget(glm::vec3 target);
    glm::vec3 GetTarget() const;

    void SetRadius(float r);
    float GetRadius() const;

    void SetPosition(glm::vec3 position);
    void SetWorldUp(glm::vec3 worldUp);

    void SetYaw(float yaw);
    void SetPitch(float pitch);

    void SetMouseSensitivity(float s);
    void SetScrollSpeed(float s);
    void SetMovementSpeed(float s);

    void SetZoom(float zoom_deg);
    float GetZoom() const;

    // --- Matrices ---
    glm::mat4 GetViewMatrix() const;

    // --- Inputs ---
    void ProcessMouseMovement(float dx, float dy, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

private:
    void UpdateCameraVectors();

private:
    // Orbit state
    glm::vec3 mTarget;
    float     mRadius;

    // Derived camera vectors
    glm::vec3 mPosition;
    glm::vec3 mFront;
    glm::vec3 mUp;
    glm::vec3 mRight;
    glm::vec3 mWorldUp;

    // Angles (degrees)
    float mYaw;
    float mPitch;

    // Speeds / params
    float mMouseSensitivity;
    float mScrollSpeed;
    float mMovementSpeed;
    float mZoom; // projection FOV
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


