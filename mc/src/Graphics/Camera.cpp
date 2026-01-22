// Camera.cpp
#include "Camera.hpp"

Camera::Camera(glm::vec3 pos) : Pos(pos), WorldUp({0,1,0}), Yaw(-90.0f), Pitch(0.0f) {
    updateVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Pos, Pos + Front, Up);
}

void Camera::ProcessKey(int dir, float dt) {
    float velocity = Speed * dt;
    if (dir == 0) Pos += Front * velocity;      // W
    if (dir == 1) Pos -= Front * velocity;      // S
    if (dir == 2) Pos -= Right * velocity;      // A
    if (dir == 3) Pos += Right * velocity;      // D
    if (dir == 4) Pos += WorldUp * velocity;    // Space
    if (dir == 5) Pos -= WorldUp * velocity;    // Ctrl
}

void Camera::ProcessMouse(float xoff, float yoff) {
    Yaw += xoff * Sens;
    Pitch += yoff * Sens;
    if (Pitch > 89.0f) Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;
    updateVectors();
}

void Camera::updateVectors() {
    glm::vec3 f;
    f.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    f.y = sin(glm::radians(Pitch));
    f.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(f);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}