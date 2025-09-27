#pragma once

#include "Graphics.hpp"

#include "Glad/glad.h"
#include <iostream>

namespace Euclid
{
class Core {
public:
    bool Init(int width, int height, int gl_major, int gl_minor);
    void CleanUp();

    void Resize(int width, int height);
    void Update(float dtSeconds);
    void Render();
    
    void InitShader();
    void UseShader();
    
    // Mouse input coming from the host via wrapper
    void OnMouseMove(double x, double y);                    // absolute window coords
    void OnMouseButton(int button, bool down, unsigned mods);
    void OnScroll(double dx, double dy);
    void OnMods(unsigned mods);
    
private:
    int mWidth;
    int mHeight;
    unsigned int mVAO = 0;
    unsigned int mVBO = 0;
    unsigned int mDummyVAO = 0;
    
    Camera mainCamera;
    float mLastMouseX = 0.f, mLastMouseY = 0.f;
    bool  mFirstMouse = true;
    float mAngleX = 0.0f, mAngleY = 0.0f;        // current
    float mTargetAngleX = 0.0f, mTargetAngleY = 0.0f; // targets
    glm::mat4 mModel = glm::mat4(1.0f);
    
    Shader mainVertex;
    Shader mainFragment;
    ShaderProgram mainShader;
    
    Shader gridVertex;
    Shader gridFragment;
    ShaderProgram gridShader;
};
}

