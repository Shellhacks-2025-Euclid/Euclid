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
    
private:
    int mWidth;
    int mHeight;
};
}

