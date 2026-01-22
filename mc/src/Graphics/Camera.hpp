// Camera.hpp
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 Pos, Front, Up, Right, WorldUp;
    float Yaw, Pitch, Speed = 10.0f, Sens = 0.1f;

    Camera(glm::vec3 position);
    glm::mat4 GetViewMatrix() const;
    void ProcessKey(int direction, float deltaTime);
    void ProcessMouse(float xoffset, float yoffset);

private:
    void updateVectors();
};