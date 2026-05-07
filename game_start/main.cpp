#include "Game.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "utils.h"
#include "Level.h"
#include "PlayerWeapon.h"



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
    ResourceManager::LoadTexture("wood", "assets/wall.jpg");
    ResourceManager::LoadTexture("youtube", "assets/youtube.png");
    ResourceManager::LoadTexture("exit", "assets/exit.png");
    ResourceManager::LoadTexture("github", "assets/github.jpg");
    ResourceManager::LoadTexture("vs", "assets/vs.png");
    ResourceManager::LoadTexture("clash", "assets/clash.png");
    ResourceManager::LoadTexture("ld", "assets/ld.png");
    ResourceManager::LoadTexture("ss", "assets/ss.png");
    
    ResourceManager::LoadCubemap("skybox", {
        "assets/skybox/sea/right.jpg", "assets/skybox/sea/left.jpg",
        "assets/skybox/sea/top.jpg", "assets/skybox/sea/bottom.jpg",
        "assets/skybox/sea/front.jpg", "assets/skybox/sea/back.jpg"
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
   
    ResourceManager::LoadModel("whale", "assets/models/animals/Whale.obj");

    ResourceManager::LoadModel("tree", "assets/scenes/pinetree/pinetree.obj");
    ResourceManager::LoadModel("plant", "assets/scenes/environoment/Plant.obj");
    ResourceManager::LoadModel("rock", "assets/scenes/environoment/Rock_01.obj");
    ResourceManager::LoadModel("greenTree", "assets/scenes/environoment/Tree_Green_01.obj");
    ResourceManager::LoadModel("redTree", "assets/scenes/environoment/WizardTree.obj");
    ResourceManager::LoadModel("trunk", "assets/scenes/environoment/Trunk_01.obj");

    


    // ==========================================
    // === 使用数据驱动关卡系统 ===
    /*
    每个关卡场景的第一次，需要先注释本块内容，使用下面的std::vector<ObjectSpawnData> 
    设定该场景需要的所有模型，初始位置、旋转、缩放等不用管,
    用地图编辑器直接摆放就行。多个同样模型需要填充多个vector数据。
    场景构建完成后，注释std::vector<ObjectSpawnData>的所有内容和遍历加载内容
    然后开启本块代码直接load加载地图json数据即可
    */
    // ==========================================
    
    Level levelManager;

    bool hasMap = levelManager.Load("assets/levels/level_01.json", &game);
    
    if (!hasMap) {
        std::cout << "No level file found, starting empty level..." << std::endl;
        exit(-1);
    }

    // ==========================================


    // 5. 初始化交互式 UI 菜单
    game.SetupMenu();


    // ==========================================
    // 6. 场景装配
    // ==========================================

    // 1. 准备数据表 
    // std::vector<ObjectSpawnData> mainObjects = {
    //     {"Bike","moto","standard","static",{2.0f, -0.49f, 2.0f},{0.0f, 0.0f, 0.0f}, {0.2f, 0.2f, 0.2f}},
    //     {"Sk2","sk2","standard","static",{0.0f, -0.49f, 0.0f},{0.0f,   0.0f, 0.0f}, {0.2f, 0.2f, 0.2f}},

    //     {"House1","house1","standard","static",{-14.0f, -0.49f,  -6.0f}, {0.0f,  90.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
    //     {"House2","house1","standard","static",{-14.0f, -0.49f,  10.0f}, {0.0f,  90.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
    //     {"House3","house2","standard","static",{ 14.0f, -0.49f,  10.0f}, {0.0f, -90.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
    //     {"House4","house2","standard","static",{ 14.0f, -0.49f,  -6.0f}, {0.0f, -90.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
    //     {"House5","house1","standard","static",{  0.0f, -0.49f, -20.0f}, {0.0f,   0.0f, 0.0f}, {2.0f, 2.0f, 2.0f}},
        
    //     {"Barrel1", "barrel1", "standard","static",{17.0f, -0.49f,  0.0f}},
    //     {"Barrel2", "barrel2", "standard","static",{16.5f, -0.49f,  0.5f}},
    //     {"Crate1",  "crate1",  "standard","static",{18.0f, -0.49f,  0.2f}},
    //     {"Shield","shield","standard","static",{-3.0f, 1.5f, 3.0f},{0.0f, 90.0f, 0.0f}, {1.2f, 1.2f, 1.2f}},
    //     {"Sword","sword","standard","static",{-3.0f, 1.5f, 1.5f},{0.0f, 90.0f, 0.0f}, {1.2f, 1.2f, 1.2f}},
    //     {"Tree","tree","standard","static",{-10.0f, -0.5f, 2.0f},{0.0f, 0.0f, 0.0f}, {5.0f, 5.0f, 5.0f}},
    //     {"Plant","plant","standard","static",{-6.0f, -0.4f, 19.0f},{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    //     {"Rock","rock","standard","static",{1.0f, -0.4f, 17.0f},{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    //     {"Trunk1","trunk","standard","static",{2.5f, -0.4f, 16.5f},{0.0f, 30.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    //     {"Trunk2","trunk","standard","static",{5.5f, -0.4f, 15.5f},{0.0f, 65.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    //     {"GreenTree1","greenTree","standard","static",{4.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     {"GreenTree2","greenTree","standard","static",{11.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     {"GreenTree3","greenTree","standard","static",{17.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     {"RedTree1","redTree","standard","static",{-17.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     {"RedTree2","redTree","standard","static",{-11.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     {"RedTree3","redTree","standard","static",{-4.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     {"Whale","whale","standard","animal",{0.0f, 10.0f, -13.5f},{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    //     // {"RedTree4","redTree","standard","static",{-4.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     // {"RedTree5","redTree","standard","static",{-4.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     // {"RedTree6","redTree","standard","static",{-4.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     // {"RedTree7","redTree","standard","static",{-4.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     // {"RedTree8","redTree","standard","static",{-4.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    //     // {"RedTree9","redTree","standard","static",{-4.0f, -0.4f, 18.0f},{0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.5f}},
    // };

    // // 2. 通用的对象创建循环 (引擎逻辑)
    // GameObject* go = nullptr;
    // for (const auto& data : mainObjects) {
    //     if (data.type == "static"){
    //         go = new GameObject(data.name, data.modelId, data.shaderId);
    //     }else if (data.type == "animal"){
    //         go = new Animal(data.name, data.modelId, data.shaderId);
    //         go->type = "animal";
    //     }
    //     go->transform.position = data.position;
    //     go->transform.rotation = data.rotation;
    //     go->transform.scale = data.scale;

    //     Model* model = ResourceManager::GetModel(go->modelName);
    //     if (model->calculateAABB){
    //         go->localAABB = model->localAABB;
    //         go->hasCollision = true; // 只有正确计算了包围盒的，才开启碰撞
    //     }  
        
    //     game.AddObject(go);
    // }


    // ResourceManager::LoadModel("gun", "assets/models/gun_pack/SMG_Full_East.obj", false);

    // GameObject* go = new GameObject("Gun", "gun", "standard");
    // go->transform.position = glm::vec3(0.2f, 2.0f, 1.0f);
    // // go->transform.rotation = glm::vec3(0.2f);
    // go->transform.scale = glm::vec3(2.0f);
    // game.AddObject(go);


    WeaponConfig m416("M416", "m416", "standard",{0.5f, -2.0f, -1.5f},{-10.0f,190.0f,0.0f},{0.2f,0.2f,0.2f});

    PlayerWeapon* m4 = new PlayerWeapon(m416, &game.camera);
    game.AddObject(m4);

    // 3. 添加具有“特殊独立逻辑”的玩家武器
    // PlayerWeapon* gun = new PlayerWeapon("M416", "m416", "standard", &game.camera);
    // gun->transform.scale = glm::vec3(0.2f); 
    // game.AddObject(gun);
 
    // 7. 启动游戏主循环！
    game.Run();

    return 0;
}
