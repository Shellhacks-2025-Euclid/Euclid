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
    
    // Gizmo Draw (0_o)
    void BuildTranslationGizmo(const glm::vec3& origin, float L);
    void BuildScaleTips(const glm::vec3& origin, float L);
    void DrawTranslationGizmo(const glm::mat4& viewProj);
    void DrawRotationGizmo(const glm::mat4& viewProj);
    void DrawTransformationGizmo(const glm::mat4& viewProj);
    
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
    unsigned int mTranslationVAO = 0;
    unsigned int mTransformationVAO = 0;
    unsigned int mTranslationVBO = 0;
    unsigned int mTransformationVBO = 0;
    
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
    
    Shader translationVertex;
    Shader translationFragment;
    ShaderProgram translationShader;
    
    Shader rotationVertex;
    Shader rotationFragment;
    ShaderProgram rotationShader;
    
    Shader transformationVertex;
    Shader transformationFragment;
    ShaderProgram transformationShader;
};
}

