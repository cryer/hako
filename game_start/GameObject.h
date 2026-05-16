#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <cmath>
#include "ResourceManager.h"
#include "AABB.h"

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
private:
    // AABB 缓存变量
    AABB _cachedGlobalAABB;
    glm::vec3 _lastPos = glm::vec3(0.0f);
    glm::vec3 _lastScale = glm::vec3(0.0f); // 初始故意给不同值触发首次计算

public:
    std::string name;
    Transform transform;
    std::string modelName;
    std::string shaderName;
    std::string type = "static";

    std::string modelPath;  // 模型文件路径（Add Model 导入的模型用于重新加载）

    AABB localAABB;  
    bool isVisible = true;
    bool isDynamic = false;
    bool hasCollision = false; // 默认不参与碰撞


    // === 生命周期与触发器属性 ===
    bool isPersistent = false; // 是否是持久对象（切关卡时不销毁，比如玩家武器）
    bool isTrigger = false;    // 是否是触发器区域（隐形，仅用于检测相交）
    std::string targetLevel;   // 触发器指向的下一关卡路径（例如 "assets/levels/level_02.json"）

    GameObject(std::string n, std::string mod, std::string shd) 
        : name(n), modelName(mod), shaderName(shd) {}
    virtual ~GameObject() = default;

    // 统一接口：获取当前世界空间 AABB
    // === 优化：全局 AABB 缓存 ===
    AABB GetWorldAABB()  {
        // 静态物体、动态物体：实时变换到世界空间
        // 只有在位置或缩放发生改变时（比如编辑器拖拽，或物体移动），才重新计算
        if (transform.position != _lastPos || transform.scale != _lastScale) {

            _cachedGlobalAABB = localAABB.GetTransformed(transform.GetMatrix());
            _lastPos = transform.position;
            _lastScale = transform.scale;
        }
        return _cachedGlobalAABB;  
    }

    virtual void Update(float deltaTime) {} // 子类可重写逻辑

    virtual void Draw(Shader* overrideShader = nullptr, int lodLevel = 0) {
        Shader* shader = overrideShader ? overrideShader : ResourceManager::GetShader(shaderName);
        if (!shader) return;
        Model* model = ResourceManager::GetModel(modelName);
        if (!model) return;
        shader->setMatrix4fv("model", transform.GetMatrix());
        model->Draw(*shader, lodLevel);
    }
};

class Animal: public GameObject{
private:
    // 移动参数（可在构造函数中自定义）
    float moveSpeed = 3.0f;      // 单位：世界单位/秒
    float moveRange = 10.0f;      // 单边移动距离，总跨度 = 10.0
    int moveDirection = 1;       // 1 = 向右(+X), -1 = 向左(-X)
public:

    Animal(std::string n, std::string mod, std::string shd) : 
          GameObject(n, mod, shd){
             isDynamic = true;
             
          }

    void Update(float deltaTime) override {
        transform.position.x += moveDirection * moveSpeed * deltaTime;

        // 边界检测 + 掉头逻辑
        if (transform.position.x >= moveRange) {
            transform.position.x = moveRange;  //  clamp到边界
            moveDirection = -1;
            transform.rotation.y = -90.0f;     // 面朝负X轴
        }
        else if (transform.position.x <= -moveRange) {
            transform.position.x = -moveRange;
            moveDirection = 1;
            transform.rotation.y = 90.0f;       // 面朝正X轴
        }
    }
};


