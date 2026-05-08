#include "Level.h"
#include "ResourceManager.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Level& Level::Instance() {
    static Level instance;
    return instance;
}

Level::~Level(){
    for (auto it = _objects.begin(); it != _objects.end(); ) {
        GameObject* obj = *it;
        delete obj; // 释放内存
        _objects.erase(it); // 从列表中移除   
    }
}


bool Level::Load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open level file: " << filepath << std::endl;
        return false;
    }

    json levelData;
    file >> levelData;

    levelName = levelData.value("name", "Unknown Level");
    if (levelData.contains("player_spawn")) {
        playerSpawn = levelData["player_spawn"].get<glm::vec3>();
    }

    // 可以在这里获取并预加载当前关卡需要的所有模型和着色器资源...
    
    // 解析物体
    if (levelData.contains("objects")) {
        for (const auto& objJson : levelData["objects"]) {
            GameObject* go = CreateObjectFromJson(objJson);
            if (go) {
                AddObject(go);
            }
        }
    }

    std::cout << "Level [" << levelName << "] loaded successfully!" << std::endl;
    return true;
}

bool Level::Save(const std::string& filepath, const glm::vec3& currentSpawn) {
    json levelData;
    levelData["name"] = levelName.empty() ? "New Level" : levelName;
    levelData["player_spawn"] = currentSpawn;

    json objectsArray = json::array();
    for (GameObject* obj : _objects) {
        // 这里可以过滤掉不需要保存的物体，比如动态生成的子弹或者玩家自己的武器
        // if (obj->name == "M416" || obj->name == "Player") continue; 
        if (obj->isPersistent) continue;
        
        objectsArray.push_back(CreateJsonFromObject(obj));
    }
    levelData["objects"] = objectsArray;

    std::ofstream file(filepath);
    if (file.is_open()) {
        file << levelData.dump(4); // 4个空格缩进，方便人类阅读
        std::cout << "Level saved to " << filepath << std::endl;
        return true;
    }
    return false;
}

void Level::Unload() {
    // 遍历场景对象，删除非持久化的对象
    for (auto it = _objects.begin(); it != _objects.end(); ) {
        GameObject* obj = *it;
        if (!obj->isPersistent) {
            delete obj; // 释放内存
            it = _objects.erase(it); // 从列表中移除
        } else {
            ++it; // 保留持久化对象
        }
    }
}

GameObject* Level::CreateObjectFromJson(const nlohmann::json& j) {
    std::string name = j.value("name", "Object");

    bool isTrigger = j.value("isTrigger", false);
    GameObject* go = nullptr;
    if (isTrigger) {
        // 触发器不需要模型和着色器
        go = new GameObject(name, "", "");
        go->isTrigger = true;
        go->hasCollision = false;
        go->targetLevel = j.value("target_level", "");
        // 给触发器一个默认大小的包围盒，后续可缩放
        go->localAABB.min = glm::vec3(-1.0f);
        go->localAABB.max = glm::vec3(1.0f);
        go->transform.position = j.value("pos", glm::vec3(0.0f));
        go->transform.rotation = j.value("rot", glm::vec3(0.0f));
        go->transform.scale = j.value("scale", glm::vec3(1.0f));
    } else {
        std::string modelId = j.value("model", "");
        std::string shaderId = j.value("shader", "standard");
        std::string type = j.value("type", "static");

        // 支持你原有的不同派生类，这就是工厂模式的雏形
        if (type == "animal") {
            go = new Animal(name, modelId, shaderId); // 如果你有Animal类
        } else {
            go = new GameObject(name, modelId, shaderId);
        }

        go->transform.position = j.value("pos", glm::vec3(0.0f));
        go->transform.rotation = j.value("rot", glm::vec3(0.0f));
        go->transform.scale = j.value("scale", glm::vec3(1.0f));

        Model* model = ResourceManager::GetModel(go->modelName);
        if (model && model->calculateAABB) {
            go->localAABB = model->localAABB;
            go->hasCollision = true;
        }
    }
    return go;
}

nlohmann::json Level::CreateJsonFromObject(GameObject* obj) {
    json j;
    j["name"] = obj->name;
    j["pos"] = obj->transform.position;
    j["rot"] = obj->transform.rotation;
    j["scale"] = obj->transform.scale;
    // 简单区分类型
    j["type"] = obj->type.c_str();
    j["isTrigger"] = obj->isTrigger;

    if (obj->isTrigger) {
        j["target_level"] = obj->targetLevel;
    } else {
        j["model"] = obj->modelName;
        j["shader"] = obj->shaderName;
        j["collision"] = obj->hasCollision;
    }

    return j;
}