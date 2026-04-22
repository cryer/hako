#pragma once
#include "GameObject.h"
#include "camera.h"

class PlayerWeapon : public GameObject {
private:
    Camera* mainCamera;
    bool isFiring = false;
    float fireStartTime = 0.0f;
    const float FIRE_DURATION = 0.2f;

    glm::mat4 finalModelMatrix; // 覆写基类的绘制矩阵

public:
    PlayerWeapon(std::string n, std::string mod, std::string shd, Camera* cam);

    void Fire(float currentTime);
    void Update(float deltaTime) override;
    void Draw(Shader* overrideShader = nullptr) override;
};