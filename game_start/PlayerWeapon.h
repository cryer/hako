#pragma once
#include "GameObject.h"
#include "camera.h"
#include "utils.h"

class PlayerWeapon : public GameObject {
private:
    Camera* mainCamera;
    bool isFiring = false;
    float fireStartTime = 0.0f;
    const float FIRE_DURATION = 0.2f;

    glm::mat4 finalModelMatrix; // 覆写基类的绘制矩阵
    WeaponConfig config;

public:
    PlayerWeapon(const WeaponConfig& cfg, Camera* cam);

    void Fire(float currentTime);
    void Update(float deltaTime) override;
    void Draw(Shader* overrideShader = nullptr, int lodLevel = 0) override;
};