#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <memory>

#include "GameObject.h"


// 方便 glm::vec3 和 json 的相互转换
namespace glm {
    inline void to_json(nlohmann::json& j, const vec3& v) {
        j = nlohmann::json{v.x, v.y, v.z};
    }
    inline void from_json(const nlohmann::json& j, vec3& v) {
        v.x = j.at(0).get<float>();
        v.y = j.at(1).get<float>();
        v.z = j.at(2).get<float>();
    }
}

class Level {
public:
    // 单例访问接口
    static Level& Instance();

    // 删除拷贝/移动
    Level(const Level&) = delete;
    Level& operator=(const Level&) = delete;
    Level(Level&&) = delete;
    Level& operator=(Level&&) = delete;

    ~Level();
    
    std::string levelName;
    glm::vec3 playerSpawn = glm::vec3(0.0f, 0.0f, 0.0f);
    // 使用unique_ptr会导致写法更繁琐一些，因为_objects需要在Game类循环中频繁使用，以及碰撞检测
    // 四叉树等，就用原始指针提供直接访问接口更清晰
    std::vector<GameObject*> _objects;
  

    // 加载关卡文件，并将生成的对象注入到 _objects 中
    bool Load(const std::string& filepath);
    
    // 将当前的 _objects 保存到关卡 JSON 文件中
    bool Save(const std::string& filepath, const glm::vec3& currentSpawn);

    void Unload();
  
    void AddObject(GameObject* obj) { _objects.push_back(obj); }

private:
    // 私有化默认构造
    Level() = default;
    GameObject* CreateObjectFromJson(const nlohmann::json& j);
    nlohmann::json CreateJsonFromObject(GameObject* obj);
};