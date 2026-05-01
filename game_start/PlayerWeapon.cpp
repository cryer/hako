#include "PlayerWeapon.h"
#include <GLFW/glfw3.h>
#include <iostream>

PlayerWeapon::PlayerWeapon(const WeaponConfig& cfg, Camera* cam)
    : GameObject(cfg.name, cfg.modelId, cfg.shaderId), mainCamera(cam), config(cfg) {
        isPersistent = true;
    }

void PlayerWeapon::Fire(float currentTime) {
    if (!isFiring) {
        isFiring = true;
        fireStartTime = currentTime;
    }
}


void PlayerWeapon::Update(float deltaTime) {
    glm::vec3 recoilOffset(0.0f);
    
    // 开火抖动逻辑
    if (isFiring) {
        float elapsedTime = static_cast<float>(glfwGetTime()) - fireStartTime;
        if (elapsedTime < FIRE_DURATION) {
            float progress = elapsedTime / FIRE_DURATION;
            float amplitude = (1.0f - progress) * 0.3f;
            float shake = sin(progress * 100.0f) * amplitude;
            recoilOffset.z = -shake * 0.5f; 
            recoilOffset.y = shake * 0.1f;  
        } else {
            isFiring = false;
        }
    }

    // 枪支位置运算
    glm::vec3 weaponOffset = config.weaponOffset;
    weaponOffset.y += recoilOffset.y;
    weaponOffset.z += recoilOffset.z;

    glm::mat4 cameraWorld = glm::inverse(mainCamera->GetViewMatrix());
    glm::vec4 worldPos = cameraWorld * glm::vec4(weaponOffset, 1.0f);
    glm::mat3 cameraRot = glm::mat3(cameraWorld);


    glm::mat4 weaponLocalRot = glm::rotate(glm::mat4(1.0f), glm::radians(config.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f))
                             * glm::rotate(glm::mat4(1.0f), glm::radians(config.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f))
                             * glm::rotate(glm::mat4(1.0f), glm::radians(config.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // 计算出跟随相机的最终矩阵
    finalModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(worldPos))
                     * glm::mat4(cameraRot)
                     * weaponLocalRot
                     * glm::scale(glm::mat4(1.0f), config.scale);
}

void PlayerWeapon::Draw(Shader* overrideShader) {
    Shader* shader = overrideShader ? overrideShader : ResourceManager::GetShader(shaderName);
    Model* model = ResourceManager::GetModel(modelName);
    if (!shader || !model) return;

    shader->setMatrix4fv("model", finalModelMatrix);
    model->Draw(*shader);
}