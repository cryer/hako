#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "ResourceManager.h"

struct Transform {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f); // 欧拉角 (度)
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 GetMatrix() const {
        glm::mat4 mat(1.0f);
        mat = glm::translate(mat, position);
        mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0.0, 1.0, 0.0));
        mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1.0, 0.0, 0.0));
        mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0.0, 0.0, 1.0));
        mat = glm::scale(mat, scale);
        return mat;
    }
};

class GameObject {
public:
    std::string name;
    Transform transform;
    std::string modelName;
    std::string shaderName;

    bool isVisible = true;

    GameObject(std::string n, std::string mod, std::string shd) 
        : name(n), modelName(mod), shaderName(shd) {}
    virtual ~GameObject() = default;

    virtual void Update(float deltaTime) {} // 子类可重写逻辑

    virtual void Draw(Shader* overrideShader = nullptr) {
        Shader* shader = overrideShader ? overrideShader : ResourceManager::GetShader(shaderName);
        if (!shader) return;
        Model* model = ResourceManager::GetModel(modelName);
        if (!model) return;

        shader->setMatrix4fv("model", transform.GetMatrix());
        model->Draw(*shader);
    }
};