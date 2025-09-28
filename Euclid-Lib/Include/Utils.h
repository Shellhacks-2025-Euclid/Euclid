#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

inline glm::mat4 TRS(const EuclidTransform& t)
{
    const glm::vec3 P(t.position[0], t.position[1], t.position[2]);
    const glm::vec3 S(t.scale[0],    t.scale[1],    t.scale[2]);

    // Euler angles are DEGREES in your Transform; convert to radians.
    const float rx = glm::radians(t.rotation[0]);
    const float ry = glm::radians(t.rotation[1]);
    const float rz = glm::radians(t.rotation[2]);

    const glm::mat4 T  = glm::translate(glm::mat4(1.0f), P);
    const glm::mat4 R  = glm::eulerAngleXYZ(rx, ry, rz);   // <<< XYZ intrinsic (matches gizmo)
    const glm::mat4 Sc = glm::scale(glm::mat4(1.0f), S);

    return T * R * Sc; // column-major GLM: point' = T * R * S * point
}
