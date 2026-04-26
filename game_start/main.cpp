#include "Game.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "utils.h"

int main() {
    // 1. 创建游戏实例
    Game game(800, 600);
    if (!game.Init("OpenGL Game")) return -1;

    // 2. 加载着色器资源 (起好名字，全局随时调取)
    ResourceManager::LoadShader("standard", "shaders/model_load.vs", "shaders/model_load.fs");
    ResourceManager::LoadShader("depth", "shaders/depth_shader.vs", "shaders/depth_shader.fs");
    ResourceManager::LoadShader("skybox", "shaders/skybox.vs", "shaders/skybox.fs");
    ResourceManager::LoadShader("floor", "shaders/floor.vs", "shaders/floor.fs");
    ResourceManager::LoadShader("light", "shaders/light.vs", "shaders/light.fs");
    ResourceManager::LoadShader("menu", "shaders/menu_shader.vs", "shaders/menu_shader.fs");

    // 3. 加载纹理与天空盒资源
    ResourceManager::LoadTexture("wood", "assets/wood.png");
    ResourceManager::LoadTexture("youtube", "assets/youtube.png");
    ResourceManager::LoadTexture("exit", "assets/exit.png");
    ResourceManager::LoadTexture("github", "assets/github.jpg");
    ResourceManager::LoadTexture("vs", "assets/vs.png");
    ResourceManager::LoadTexture("clash", "assets/clash.png");
    ResourceManager::LoadTexture("ld", "assets/ld.png");
    ResourceManager::LoadTexture("ss", "assets/ss.png");
    
    ResourceManager::LoadCubemap("skybox", {
        "assets/skybox/water/right.jpg", "assets/skybox/water/left.jpg",
        "assets/skybox/water/top.jpg", "assets/skybox/water/bottom.jpg",
        "assets/skybox/water/front.jpg", "assets/skybox/water/back.jpg"
    });

    // 4. 加载 3D 模型
    ResourceManager::LoadModel("moto", "assets/models/motobike/moto.pmx");
    ResourceManager::LoadModel("sk2", "assets/models/sk/sk2.pmx");
    ResourceManager::LoadModel("m416", "assets/models/m16/m416.pmx", false);

    ResourceManager::LoadModel("house1", "assets/scenes/MedievalHouse/House.obj");
    ResourceManager::LoadModel("house2", "assets/scenes/house1/medieval_house.obj");

    ResourceManager::LoadModel("barrel1", "assets/scenes/Village/Prop_Barrel_1.obj");
    ResourceManager::LoadModel("barrel2", "assets/scenes/Village/Prop_Barrel_1_Open.obj");
    ResourceManager::LoadModel("crate1", "assets/scenes/Village/Prop_Crate_1.obj");
    ResourceManager::LoadModel("crate2", "assets/scenes/Village/Prop_Crate_1_Open.obj");

    ResourceManager::LoadModel("shield", "assets/models/weapons/shield_d.obj");
    ResourceManager::LoadModel("sword", "assets/models/weapons/sword_j.obj");
    ResourceManager::LoadModel("tree", "assets/scenes/pinetree/pinetree.obj");
   
    ResourceManager::LoadModel("whale", "assets/models/animals/Whale.obj");


    

    // 5. 初始化交互式 UI 菜单
    game.SetupMenu();

    // ==========================================
    // 6. 场景装配
    // ==========================================

    // 1. 准备数据表 
    std::vector<ObjectSpawnData> staticObjects = {
        {"Bike", "moto", "standard",{2.0f, -0.49f, 2.0f},{0.0f, 0.0f, 0.0f}, {0.2f, 0.2f, 0.2f}},
        {"Sk2", "sk2", "standard",{0.0f, -0.49f, 0.0f},{0.0f,   0.0f, 0.0f}, {0.2f, 0.2f, 0.2f}},

        // 房子组
        {"House1", "house1", "standard", {-14.0f, -0.49f,  -6.0f}, {0.0f,  90.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
        {"House2", "house1", "standard", {-14.0f, -0.49f,  10.0f}, {0.0f,  90.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
        {"House3", "house2", "standard", { 14.0f, -0.49f,  10.0f}, {0.0f, -90.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
        {"House4", "house2", "standard", { 14.0f, -0.49f,  -6.0f}, {0.0f, -90.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
        {"House5", "house1", "standard", {  0.0f, -0.49f, -20.0f}, {0.0f,   0.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
        
        // 杂物组 
        {"Barrel1", "barrel1", "standard", {17.0f, -0.49f,  0.0f}},
        {"Barrel2", "barrel2", "standard", {16.5f, -0.49f,  0.5f}},
        {"Crate1",  "crate1",  "standard", {18.0f, -0.49f,  0.2f}},
        {"Shield","shield","standard",{-3.0f, 1.5f, 3.0f},{0.0f, 90.0f, 0.0f}, {1.2f, 1.2f, 1.2f}},
        {"Sword","sword","standard",{-3.0f, 1.5f, 1.5f},{0.0f, 90.0f, 0.0f}, {1.2f, 1.2f, 1.2f}},
        {"Tree","tree","standard",{-10.0f, -0.5f, 2.0f},{0.0f, 0.0f, 0.0f}, {5.0f, 5.0f, 5.0f}},
    };

    std::vector<ObjectSpawnData> moveObjects = {
        {"Whale","whale","standard",{0.0f, 10.0f, -13.5f},{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    };
    // 2. 通用的对象创建循环 (引擎逻辑)
    for (const auto& data : staticObjects) {
        GameObject* go = new GameObject(data.name, data.modelId, data.shaderId);
        go->transform.position = data.position;
        go->transform.rotation = data.rotation;
        go->transform.scale = data.scale;

        Model* model = ResourceManager::GetModel(go->modelName);
        if (model->calculateAABB){
            go->localAABB = model->localAABB;
            go->hasCollision = true; // 只有正确计算了包围盒的，才开启碰撞
        }  
        
        game.AddObject(go);
    }
    
    for (const auto& data : moveObjects) {
        GameObject* go = new Animal(data.name, data.modelId, data.shaderId);
        go->transform.position = data.position;
        go->transform.rotation = data.rotation;
        go->transform.scale = data.scale;

        Model* model = ResourceManager::GetModel(go->modelName);
        if (model->calculateAABB){
            go->localAABB = model->localAABB;
            go->hasCollision = true; // 只有正确计算了包围盒的，才开启碰撞
        } 
        // go->isDynamic = true;
        game.AddObject(go);
    }

    // 3. 添加具有“特殊独立逻辑”的玩家武器
    PlayerWeapon* gun = new PlayerWeapon("M416", "m416", "standard", &game.camera);
    gun->transform.scale = glm::vec3(0.2f); 
    game.AddObject(gun);
 
    // 7. 启动游戏主循环！
    game.Run();

    return 0;
}
