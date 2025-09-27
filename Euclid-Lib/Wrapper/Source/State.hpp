#pragma once
#include "Core.hpp"
#include "Renderer.hpp"

struct EuclidState {
    Euclid::Core core;
    int  fbW = 0, fbH = 0;
    bool ready = false;

    unsigned int targetFbo = 0; // 0 = default/backbuffer
};
