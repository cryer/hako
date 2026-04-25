#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>


// 定义场景物体的数据结构
struct ObjectSpawnData {
    std::string name;
    std::string modelId;
    std::string shaderId;
    glm::vec3 position;
    glm::vec3 rotation; // 欧拉角 (Pitch, Yaw, Roll)
    glm::vec3 scale;

    // 构造函数，方便快速填写数据，默认缩放为 1，旋转为 0
    ObjectSpawnData(std::string n, 
                    std::string m, 
                    std::string s, 
                    glm::vec3 pos, 
                    glm::vec3 rot = glm::vec3(0.0f), 
                    glm::vec3 sca = glm::vec3(1.0f))
        : name(n), modelId(m), shaderId(s), position(pos), rotation(rot), scale(sca) {}
};