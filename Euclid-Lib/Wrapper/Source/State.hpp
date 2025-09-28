#pragma once
#include "Core.hpp"
#include "Renderer.hpp"
#include <unordered_map>

namespace Euclid {
struct Object; // fwd
}

struct EuclidState {
    Euclid::Core core;
    int  fbW = 0, fbH = 0;
    bool ready = false;
    unsigned int targetFbo = 0; // 0 = default/backbuffer

    // Scene registry (ID -> Object)
    std::unordered_map<EuclidObjectID, std::unique_ptr<Euclid::Object>> objects;
    EuclidObjectID nextID = 1;
    EuclidObjectID selected = 0;

    EuclidGizmoMode gizmoMode = EUCLID_GIZMO_TRANSLATE;
};
