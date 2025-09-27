#include "Graphics.hpp"

namespace Euclid
{

Camera::Camera(glm::vec3 target, float radius)
    : mTarget(target)
    , mRadius(glm::max(radius, 0.1f))
    , mPosition({0.0f, 0.0f, 0.0f})
    , mFront({0.0f, 0.0f, -1.0f})
    , mUp({0.0f, 1.0f, 0.0f})
    , mRight()
    , mWorldUp({0.0f, 1.0f, 0.0f})
    , mYaw(-90.0f)
    , mPitch(0.0f)
    , mMouseSensitivity(0.1f)
    , mScrollSpeed(0.5f)
    , mMovementSpeed(2.5f)
    , mZoom(45.0f)
{
    UpdateCameraVectors();
}

Camera::~Camera() {}

void Camera::SetTarget(glm::vec3 target) {
    mTarget = target;
    UpdateCameraVectors();
}

glm::vec3 Camera::GetTarget() const { return mTarget; }

void Camera::SetRadius(float r) {
    mRadius = glm::max(r, 0.1f);
    UpdateCameraVectors();
}

float Camera::GetRadius() const { return mRadius; }

void Camera::SetPosition(glm::vec3 position) {
    mPosition = position;
    mRadius = glm::max(glm::length(mPosition - mTarget), 0.1f);

    glm::vec3 dir = glm::normalize(mTarget - mPosition);
    mPitch = glm::degrees(std::asin(glm::clamp(dir.y, -1.0f, 1.0f)));
    mYaw   = glm::degrees(std::atan2(dir.z, dir.x));
    UpdateCameraVectors();
}

void Camera::SetWorldUp(glm::vec3 worldUp) {
    mWorldUp = worldUp;
    UpdateCameraVectors();
}

void Camera::SetYaw(float yaw) {
    mYaw = yaw;
    UpdateCameraVectors();
}

void Camera::SetPitch(float pitch) {
    mPitch = glm::clamp(pitch, -89.0f, 89.0f);
    UpdateCameraVectors();
}

void Camera::SetMouseSensitivity(float s) { mMouseSensitivity = s; }
void Camera::SetScrollSpeed(float s)      { mScrollSpeed = s; }
void Camera::SetMovementSpeed(float s)    { mMovementSpeed = s; }

void Camera::SetZoom(float zoom_deg) {
    mZoom = glm::clamp(zoom_deg, 1.0f, 90.0f);
}

float Camera::GetZoom() const { return mZoom; }

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(mPosition, mTarget, mUp);
}

void Camera::ProcessMouseMovement(float dx, float dy, bool constrainPitch) {
    dx *= mMouseSensitivity;
    dy *= mMouseSensitivity;

    mYaw   += dx;
    mPitch -= dy;

    if (constrainPitch) {
        if (mPitch > 89.0f)  mPitch = 89.0f;
        if (mPitch < -89.0f) mPitch = -89.0f;
    }
    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    mRadius += yoffset * mScrollSpeed;
    if (mRadius < 0.1f) mRadius = 0.1f;
    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors() {
    float yawRad   = glm::radians(mYaw);
    float pitchRad = glm::radians(mPitch);

    float cp = std::cos(pitchRad);
    glm::vec3 offset{
        mRadius * std::cos(yawRad) * cp,
        mRadius * std::sin(pitchRad),
        mRadius * std::sin(yawRad) * cp
    };

    mPosition = mTarget + offset;

    mFront = glm::normalize(mTarget - mPosition);
    mRight = glm::normalize(glm::cross(mFront, mWorldUp));
    mUp    = glm::normalize(glm::cross(mRight, mFront));
}

}
