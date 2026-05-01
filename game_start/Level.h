#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "GameObject.h"


class Game;

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
    std::string levelName;
    glm::vec3 playerSpawn = glm::vec3(0.0f, 0.0f, 0.0f);

    // 加载关卡文件，并将生成的对象注入到 game->sceneObjects 中
    bool Load(const std::string& filepath, Game* game);
    
    // 将当前的 sceneObjects 保存到关卡 JSON 文件中
    bool Save(const std::string& filepath, const std::vector<GameObject*>& objects, const glm::vec3& currentSpawn);

    void Unload(Game* game);

private:
    GameObject* CreateObjectFromJson(const nlohmann::json& j);
    nlohmann::json CreateJsonFromObject(GameObject* obj);
};