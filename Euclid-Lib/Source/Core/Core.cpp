#include "Core.hpp"

namespace Euclid {
void Init() {
    
}
bool Core::Init(int width, int height, int gl_major, int gl_minor) {
    mWidth = width;
    mHeight = height;
    
    // 3) Create VAO/VBO
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // 4) Basic state
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    
    return true;
}
void Core::CleanUp() {
    if (mVBO) { glDeleteBuffers(1, &mVBO); mVBO = 0; }
    if (mVAO) { glDeleteVertexArrays(1, &mVAO); mVAO = 0; }
}
void Core::Resize(int width, int height) {
    mWidth = width;
    mHeight = height;
    glViewport(0, 0, width, height);
}
void Core::Update(float dtSeconds) {
    
}
void Core::Render() {
    glClearColor(0.1f, 0.1f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindVertexArray(mVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
void Core::OnMouseMove(double x, double y) {
    
}
void Core::OnMouseButton(int button, bool down, unsigned mods) {
    
}
void Core::OnScroll(double dx, double dy) {
    
}
}

