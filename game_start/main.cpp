#include "Game.h"
#include "ResourceManager.h"

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
    ResourceManager::LoadTexture("youtube", "assets/youtube.jpg");
    ResourceManager::LoadTexture("exit", "assets/exit.jpg");
    ResourceManager::LoadTexture("github", "assets/github.jpg");
    
    ResourceManager::LoadCubemap("skybox", {
        "assets/skybox/water/right.jpg", "assets/skybox/water/left.jpg",
        "assets/skybox/water/top.jpg", "assets/skybox/water/bottom.jpg",
        "assets/skybox/water/front.jpg", "assets/skybox/water/back.jpg"
    });

    // 4. 加载 3D 模型
    ResourceManager::LoadModel("moto", "assets/models/motobike/moto.pmx");
    ResourceManager::LoadModel("sk2", "assets/models/sk/sk2.pmx");
    ResourceManager::LoadModel("m416", "assets/models/m16/m416.pmx");

    // 5. 初始化交互式 UI 菜单
    game.SetupMenu();

    // ==========================================
    // 6. 场景装配：在这里随心所欲地添加你的游戏物体！
    // ==========================================
    
    // 添加摩托车
    GameObject* bike = new GameObject("Bike", "moto", "standard");
    bike->transform.position = glm::vec3(2.0f, -0.49f, 2.0f);
    bike->transform.scale = glm::vec3(0.2f);
    game.AddObject(bike);

    // 添加背包/人物
    GameObject* character = new GameObject("Sk2", "sk2", "standard");
    character->transform.position = glm::vec3(0.0f, -0.49f, 0.0f);
    character->transform.scale = glm::vec3(0.2f);
    game.AddObject(character);

    // 添加具有“特殊独立逻辑”的玩家武器
    PlayerWeapon* gun = new PlayerWeapon("M416", "m416", "standard", &game.camera);
    gun->transform.scale = glm::vec3(0.2f); 
    game.AddObject(gun);

    // ==========================================
    
    // 7. 启动游戏主循环！
    game.Run();

    return 0;
}
