#include <windows.h>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>
#include <execution>
#include <unordered_map>
#include <random>

#include "Game.h"
#include "ResourceManager.h"
#include "PlayerWeapon.h"


// 全局静态指针，用于回调函数访问
static Game* g_Game = nullptr; 

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (g_Game) { g_Game->width = width; g_Game->height = height; }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!g_Game || g_Game->terminal.GetVisible() || LevelEditor::Instance().isVisible) return;
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (g_Game->firstMouse) { g_Game->lastX = xpos; g_Game->lastY = ypos; g_Game->firstMouse = false; }
    float xoffset = xpos - g_Game->lastX;
    float yoffset = g_Game->lastY - ypos; 
    g_Game->lastX = xpos;
    g_Game->lastY = ypos;
    g_Game->camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!g_Game || g_Game->terminal.GetVisible() || LevelEditor::Instance().isVisible) return;
    g_Game->camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// 调用系统 API 启动 Chrome 并打开指定 HTTPS 网页
bool OpenChromeBrowser(const std::wstring& url) {
    if (url.empty()) return false;

    // 显式调用 chrome.exe，参数为目标 URL
    HINSTANCE hResult = ShellExecuteW(
        nullptr,
        L"open",
        L"chrome.exe",      // 浏览器可执行文件名
        url.c_str(),        // 命令行参数
        nullptr,
        SW_SHOWNORMAL       // 窗口正常显示
    );

    // ShellExecute 返回值 <= 32 表示失败（MSDN 标准错误码）
    if (reinterpret_cast<intptr_t>(hResult) <= 32) {
        std::wcerr << L"[错误] 无法启动 Chrome，错误码: " 
                   << reinterpret_cast<intptr_t>(hResult) << std::endl;
        return false;
    }
    return true;
}


Game::Game(int w, int h) : width(w), height(h), 
                           camera(glm::vec3(0.0f, 3.0f, 3.0f)), 
                           myMenu(1.5f, -10.0f),
                           sceneTree(0, {-1000.0f, -1000.0f, 1000.0f, 1000.0f}),
                           limiter(144){
    g_Game = this;
    lastX = w / 2.0f;
    lastY = h / 2.0f;

}

Game::~Game() {
    ResourceManager::Clear();

    audio.shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

MenuItem Game::CreateMenuItem(
    const std::string& name,
    const glm::vec3& pos,
    const glm::vec2& size,
    const std::string& textureName,
    std::function<void()> onClickCallback,
    float zDepth) 
{
    MenuItem item;
    item.name = name;
    // 设置位置和大小
    item.localPosition = pos;
    item.localPosition.z = zDepth; // 允许自定义Z轴，默认为0
    item.size = size;
    // 统一设置颜色风格 (可以根据需要修改默认色)
    item.defaultColor = glm::vec3(1.0f, 1.0f, 1.0f);
    item.highlightColor = glm::vec3(0.2f, 0.8f, 0.2f);
    // 统一处理纹理 (如果纹理名为空，则不设置纹理，仅显示颜色块)
    if (!textureName.empty()) {
        item.textureID = ResourceManager::GetTexture(textureName);
        item.highlightTexID = ResourceManager::GetTexture(textureName);
    } else {
        item.textureID = 0;
        item.highlightTexID = 0;
    }
    // 设置回调
    item.onClick = onClickCallback;
    return item;
}

void Game::SetupMenu() {
    myMenu.Init();    
    // 1. 添加背景 (特殊处理)
    MenuItem bgBoard;
    bgBoard.name = "BackgroundBoard";
    bgBoard.localPosition = glm::vec3(0.0f, 0.0f, -0.02f); 
    bgBoard.size = glm::vec2(1.4f, 1.0f); 
    bgBoard.defaultColor = glm::vec3(0.15f, 0.15f, 0.18f); 
    bgBoard.highlightColor = glm::vec3(0.15f, 0.15f, 0.18f); 
    myMenu.AddItem(bgBoard);
    // 2. 定义按钮配置结构体 (局部结构体或类成员)
    struct ButtonConfig {
        std::string name;
        glm::vec3 pos;
        glm::vec2 size;
        std::string tex;
        std::function<void()> action;
    };

    // 3. 初始化按钮列表
    // 注意：对于需要访问成员变量的lambda，必须在列表初始化时捕获[this]
    std::vector<ButtonConfig> buttons = {
        {"ExitGame", {0.55f, 0.35f, 0.f}, {0.2f, 0.2f}, "exit",[](){ std::cout << "Exit\n"; exit(0);}},
        {"YouTube", {-0.55f, 0.35f, 0.f},{0.25f, 0.2f},"youtube",[](){OpenChromeBrowser(L"https://www.youtube.com/");std::cout << "YouTube\n"; }},
        {"VS", {-0.55f, 0.15f, 0.f},{0.25f, 0.2f},"vs", [](){ std::cout << "VS\n"; }},
        {"Clash", {-0.55f, -0.05f, 0.f},{0.25f, 0.2f},"clash", [](){ std::cout << "Clash\n"; }},
        {"LD", {-0.55f, -0.25f, 0.f},{0.25f, 0.2f},"ld", [this](){ lightRotate = !lightRotate; std::cout << "LD\n"; }},
        {"SS", {-0.55f, -0.45f, 0.f},{0.25f, 0.2f},"ss", [this](){ shadowOn = !shadowOn; std::cout << "SS\n"; }},
        {"GitHub", {0.12f, -0.15f, 0.f},{1.0f, 0.7f}, "github", [](){ std::cout << "Git\n"; }}
    };

    // 4. 循环创建
    for (const auto& cfg : buttons) {
        myMenu.AddItem(CreateMenuItem(cfg.name, cfg.pos, cfg.size, cfg.tex, cfg.action));
    }
}

bool Game::Init(const char* title) {
    // 1. 初始化 GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // 禁用垂直同步
    glfwSwapInterval(0);  // 0=禁用VSync, 1=启用

    // 2. 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }


    // 开启深度测试 和 面剔除
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // 3. 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 4. 绑定终端指令和变量
    terminal.BindBool("showGun", &showGun);
    terminal.BindBool("shadowOn", &shadowOn);
    terminal.BindBool("lightRotate", &lightRotate);
    terminal.BindBool("showBox", &showBox);

    terminal.BindFloat("terrainMaxHeight",  &terrain.config.maxHeight);
    terminal.BindFloat("terrainNoiseScale", &terrain.config.noiseScale);
    terminal.BindInt("terrainOctaves",      &terrain.config.octaves);
    terminal.BindInt("terrainSeed",         &terrain.config.seed);

    terminal.BindFloat("windStrenth",  &windStrenth);
    terminal.BindFloat("windSpeed", &windSpeed);

    terminal.RegisterCommand("regen_terrain", [&](const std::vector<std::string>&) {
        needTerrainRegen = true;
        terminal.AddLog("Terrain regeneration scheduled.");
    }, "Apply terrain config changes");

    terminal.RegisterCommand("reset", [&](const std::vector<std::string>& args) {
        showGun = false;
        shadowOn = false;
        lightRotate = false;
        showBox = false;
        terminal.AddLog("reset light pos.");
    }, "Reset light to default");

    terminal.AddLog("Developer Terminal Ready. Press '~' to toggle.");

    // 初始化渲染器
    renderer = std::make_unique<Renderer>();

    // 初始化地板碰撞体 (Y=-0.5 表面, 30x30范围)
    floorCollider = AABB(glm::vec3(-30.0f, -1.0f, -30.0f), glm::vec3(30.0f, -0.5f, 30.0f));

    // if (!audio.init()) {
    //     std::cerr << "Running without audio.\n";
    // }

    if (!audio.init()) {
        std::cerr << "Running without audio.\n";
    } else {
        // 在资源加载阶段预加载 BGM（避免首次播放卡顿）
        audio.play_bgm("assets/sound/am.mp3", 0.1f);
    }
    
    return true;
}

void Game::ProcessInput() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (godMode) {
        // 上帝模式：自由飞行
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    } else {
        // 物理模式：水平面移动 + 跳跃
        glm::vec3 moveDir(0.0f);
        glm::vec3 frontXZ = glm::vec3(camera.Front.x, 0.0f, camera.Front.z);
        glm::vec3 rightXZ = glm::vec3(camera.Right.x, 0.0f, camera.Right.z);
        if (glm::length(frontXZ) > 0.001f) frontXZ = glm::normalize(frontXZ);
        if (glm::length(rightXZ) > 0.001f) rightXZ = glm::normalize(rightXZ);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += frontXZ;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= frontXZ;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= rightXZ;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += rightXZ;

        if (glm::length(moveDir) > 0.0f) {
            moveDir = glm::normalize(moveDir);
            playerVelocity.x = moveDir.x * camera.MovementSpeed;
            playerVelocity.z = moveDir.z * camera.MovementSpeed;
        } else {
            playerVelocity.x = 0.0f;
            playerVelocity.z = 0.0f;
        }

        // 跳跃
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && playerOnGround) {
            playerVelocity.y = 10.0f;
        }
    }

    // 触发武器开火
    if (showGun && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        for (auto obj : Level::Instance()._objects) {
            PlayerWeapon* weapon = dynamic_cast<PlayerWeapon*>(obj);
            if (weapon) {
                weapon->Fire(static_cast<float>(glfwGetTime()));
            }
        }
    }

    // 发射物理球体
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && projectileCooldown <= 0.0f) {
        Projectile p;
        p.position = camera.Position + camera.Front * 0.5f;
        p.velocity = camera.Front * 30.0f;
        p.radius = 0.25f;
        p.restitution = 0.5f;
        p.gravity = 20.0f;
        p.airDrag = 0.3f;
        p.rollFriction = 4.0f;
        p.lifetime = projectileLife;
        projectiles.push_back(p);
        projectileCooldown = PROJECTILE_COOLDOWN_TIME;
    }
}

void Game::SetupForLevel() {
    if (Level::Instance().hasTerrain) {
        if (!useTerrain) {
            Terrain::Config cfg{
                .worldSize  = 200.0f,
                .resolution = 256,
                .maxHeight  = 8.0f,
                .noiseScale = 0.02f,
                .octaves    = 4,
                .seed       = 42
            };
            // C++20聚合体的指定初始化，不支持嵌套，因此单独赋值
            cfg.pool.enabled = true;
            cfg.pool.vertices = {
                                    {28.0f, 28.0f},
                                    {36.0f, 26.0f},
                                    {42.0f, 30.0f},
                                    {46.0f, 36.0f},
                                    {44.0f, 42.0f},
                                    {38.0f, 46.0f},
                                    {30.0f, 44.0f},
                                    {24.0f, 38.0f},
                                    {22.0f, 32.0f},
                                    {26.0f, 28.0f}
                                };
            cfg.pool.floorHeight = 1.5f; // 池底高度
            cfg.pool.edgeRadius = 2.5f; // 边缘平滑过渡半径

            terrain.Generate(cfg);
            useTerrain = true;
        }

        // ====== 水面生成 ======
        {
            WaterManager::Config wcfg{
                // 水面多边形形状和上面水池地形一致，达到完整填充
                .poolVertices = {
                                    {28.0f, 28.0f},
                                    {36.0f, 26.0f},
                                    {42.0f, 30.0f},
                                    {46.0f, 36.0f},
                                    {44.0f, 42.0f},
                                    {38.0f, 46.0f},
                                    {30.0f, 44.0f},
                                    {24.0f, 38.0f},
                                    {22.0f, 32.0f},
                                    {26.0f, 28.0f}
                                },
                .waterHeight = 3.3f, // 水面高度
                .gridRes = 64 // 水面网格res
            };
  
            water.Generate(wcfg);
            std::cout << "Water pool generated." << std::endl;
        }

        // ====== 草地自动生成 ======
        {
            GrassManager::Config gcfg{
                .density        = 0.2f,
                .slopeThreshold = 0.55f,
                .stepSize       = 0.8f,
                .jitterRadius   = 0.35f,
                .minScale       = 5.4f,
                .maxScale       = 7.5f,
                .seed           = 42
            };

            grass.Generate(terrain, gcfg);
            if (grass.GetObject()) {
                Level::Instance().AddObject(grass.GetObject());
            }
            std::cout << "Grass instances: " << grass.GetInstanceCount() << std::endl;
        }

        // ====== 树木随机放置（网格防重叠） ======
        {
            float cellSize = 10.0f;
            float halfSize = terrain.config.worldSize * 0.5f;
            int gridRes = (int)(terrain.config.worldSize / cellSize);

            std::vector<std::vector<bool>> occupied(gridRes, std::vector<bool>(gridRes, false));

            struct TreeDef { std::string model; float weight; float minS; float maxS; };
            std::vector<TreeDef> treeDefs = {
                {"greenTree", 0.35f, 0.8f, 1.3f},
                {"redTree",   0.65f, 0.8f, 1.3f},
            };

            std::unordered_map<std::string, GameObject*> treeMap;
            auto ensureObj = [&](const std::string& modelName) -> GameObject* {
                auto it = treeMap.find(modelName);
                if (it != treeMap.end()) return it->second;
                auto* obj = new GameObject("Trees_" + modelName, modelName, "instanced_standard");
                obj->useInstancing = true;
                obj->castShadow = true;
                obj->hasCollision = false;
                obj->localAABB.min = glm::vec3(-halfSize, -10.0f, -halfSize);
                obj->localAABB.max = glm::vec3(halfSize, 30.0f, halfSize);
                treeMap[modelName] = obj;
                return obj;
            };

            // 局部随机引擎，线程安全，无全局副作用
            std::mt19937 rng(43);
            std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
            std::uniform_real_distribution<float> distJitter(-1.0f, 1.0f);

            for (int gx = 0; gx < gridRes; gx++) {
                for (int gz = 0; gz < gridRes; gz++) {
                    if (occupied[gx][gz]) continue;
                    // 生成树的概率
                    if (dist01(rng) > 0.85f) continue;

                    float cx = -halfSize + (gx + 0.5f) * cellSize;
                    float cz = -halfSize + (gz + 0.5f) * cellSize;
                    float ox = cx + distJitter(rng) * cellSize * 0.35f;
                    float oz = cz + distJitter(rng) * cellSize * 0.35f;

                    if (terrain.IsInsidePool(ox, oz)) continue;

                    float h = terrain.GetHeight(ox, oz);
                    glm::vec3 n = terrain.GetNormal(ox, oz);
                    if (glm::dot(n, glm::vec3(0.0f, 1.0f, 0.0f)) < 0.7f) continue;

                    // 标记占用格子（自身 + 8邻域）
                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dz = -1; dz <= 1; dz++) {
                            int nx = gx + dx, nz = gz + dz;
                            if (nx >= 0 && nx < gridRes && nz >= 0 && nz < gridRes)
                                occupied[nx][nz] = true;
                        }
                    }

                    // 加权随机选树种
                    float r = dist01(rng);
                    float acc = 0.0f;
                    int picked = 0;
                    for (int t = 0; t < (int)treeDefs.size(); t++) {
                        acc += treeDefs[t].weight;
                        if (r <= acc) { picked = t; break; }
                    }

                    glm::mat4 mat(1.0f);
                    mat = glm::translate(mat, glm::vec3(ox, h, oz));
                    mat = glm::rotate(mat, glm::radians(dist01(rng) * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    float s = treeDefs[picked].minS + dist01(rng) * (treeDefs[picked].maxS - treeDefs[picked].minS);
                    mat = glm::scale(mat, glm::vec3(s));

                    ensureObj(treeDefs[picked].model)->instances.push_back(mat);
                }
            }

            int totalTrees = 0;
            for (auto& [name, obj] : treeMap) {
                if (!obj->instances.empty()) {
                    Level::Instance().AddObject(obj);
                    totalTrees += (int)obj->instances.size();
                } else {
                    delete obj;
                }
            }
            std::cout << "Tree instances: " << totalTrees << std::endl;
        }
    } else {
        useTerrain = false;
    }
}

void Game::Run() {
    double fpsLastTime = glfwGetTime();
    int frameCount = 0;
    // 消抖 (比glfwGetTime()计时要高效)
    static bool mKeyPressed = false;
    static bool eKeyPressed = false;
    static bool qKeyPressed = false;
    static bool pKeyPressed = false;
    static bool gKeyPressed = false;

    while (!glfwWindowShouldClose(window)) {

        // 帧率限制器
        // limiter.wait();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // FPS 计算
        frameCount++;
        if (currentFrame - fpsLastTime >= 0.5) {
            double fps = double(frameCount) / (currentFrame - fpsLastTime);
            glfwSetWindowTitle(window, ("FPS: " + std::to_string(int(fps))).c_str());
            frameCount = 0;
            fpsLastTime = currentFrame;
        }         
    
        // 音频更新（清理已结束的音效）
        audio.update();    


        // 处理终端导致的鼠标状态变更
        bool currentTerminalVisible = terminal.GetVisible();
        if (currentTerminalVisible != lastTerminalVisible) {
            if (currentTerminalVisible) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                firstMouse = true; 
            }
            lastTerminalVisible = currentTerminalVisible;
        }

        // 处理地图编辑器造成的的鼠标状态变更
        bool currentEditorVisible = LevelEditor::Instance().isVisible;
        if (currentEditorVisible != lastEditorVisible) {
            if (currentEditorVisible) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                firstMouse = true; 
            }
            lastEditorVisible = currentEditorVisible;
        }
        

        myMenu.Update(camera, deltaTime);


        // --- 玩家输入 ---
        if (!currentTerminalVisible) {
            ProcessInput();

            // --- GazeMenu 输入与逻辑 ---
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
                if (!qKeyPressed) { myMenu.Toggle(camera); qKeyPressed = true; }
            } else qKeyPressed = false;

            if (myMenu.IsActive()){
                if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
                    if (!eKeyPressed) { 
                        myMenu.Interact(); eKeyPressed = true; 
                        audio.play_sfx("assets/sound/MenuSelectionClick.wav", 0.8f);
                    }
                } else eKeyPressed = false;
            }

            // 切换地图编辑器状态(消抖)
            if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
                if (!mKeyPressed) { LevelEditor::Instance().isVisible = !LevelEditor::Instance().isVisible; mKeyPressed = true; }
            } else mKeyPressed = false;

            // 暂停bgm
            if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
                if (!pKeyPressed){
                    isBgmPause = !isBgmPause;
                    audio.pause_bgm(isBgmPause);
                    pKeyPressed = true; // 这样长按就只会执行一次
                }
            } else pKeyPressed = false;

            // 切换上帝模式/物理模式 (G键)
            if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
                if (!gKeyPressed) {
                    godMode = !godMode;
                    gKeyPressed = true;
                    if (!godMode) {
                        playerVelocity = glm::vec3(0.0f);
                        terminal.AddLog("Switched to Physics Mode");
                    } else {
                        terminal.AddLog("Switched to God Mode");
                    }
                }
            } else gKeyPressed = false;
        }
        

        // ==========================================
        // 每帧重建四叉树
        // ==========================================
        sceneTree.Clear();
        for (auto obj : Level::Instance()._objects) {
            sceneTree.Insert(obj);
        }

        // 物理模式：施加重力并应用速度
        if (!godMode) {
            playerVelocity.y -= 20.0f * deltaTime;
            camera.Position += playerVelocity * deltaTime;
        }

        // 玩家碰撞box：摄像机位于box顶部(眼睛位置)，box向下延伸2.5单位
        // 直接每帧创建一个对象就好 没什么性能消耗
        playerBox = AABB(
            glm::vec3(camera.Position.x - 0.25f, camera.Position.y - 2.5f, camera.Position.z - 0.25f),
            glm::vec3(camera.Position.x + 0.25f, camera.Position.y, camera.Position.z + 0.25f));

        // ==========================================
        // 物理与触发器检测 (从遍历全部，变成只查附近)
        // ==========================================
        // 将玩家包围盒转为 2D 矩形
        Rect2D playerRect = {playerBox.min.x, playerBox.min.z, playerBox.max.x, playerBox.max.z};
        std::vector<GameObject*> nearPlayerObjects;
        sceneTree.Query(playerRect, nearPlayerObjects);

        std::string levelToLoad = "";
        
        // --- 游戏逻辑更新 (串行更新) ---
        // for (auto& obj : Level::Instance()._objects) {
        
        //     // 同步全局 showGun 状态给具体武器
        //     if (obj->name == "M416") {
        //         obj->isVisible = showGun;
        //     }
          
        //     // 只有当物体激活/可见时，才执行它的逻辑更新
        //     // 当物体多的，Update变复杂的时候，且各自的Update只修改自身的状态的时候
        //     // Update更新最好使用并行，利用C++17的execution库的for_each即可
        //     // 不过需要先把showGun先单独提取出去，防止并行修改同一变量
        //     if (obj->isVisible) {
        //         obj->Update(deltaTime);
        //     }
        // }
        auto& objects = Level::Instance()._objects;
        // 串行同步 showGun
        for (auto* obj : objects) {
            if (obj->name == "M416")
                obj->isVisible = showGun;
        }

        // 并行更新[object少于1，200可能没什么变化，但至少没有副作用]
        std::for_each(std::execution::par, objects.begin(), objects.end(),
            [this](GameObject* obj) {
                if (obj && obj->isVisible)
                    obj->Update(deltaTime);
            });

        // 更新逻辑还是全局，只有碰撞检测用四叉树查询附近
        for (auto obj : nearPlayerObjects) {
            if (obj->isTrigger) {
                // 如果玩家的 AABB 和 触发器的 AABB 相交
                if (playerBox.Intersects(obj->GetWorldAABB())) {
                    levelToLoad = obj->targetLevel;
                    break; // 找到目标立刻跳出循环，因为我们要切换关卡了
                }
            }

            if (!obj->hasCollision) continue; // 直接跳过没有碰撞的物体
            /*
            ResolveCollision内部碰撞会反推第一个参数的box坐标
            利用反推box坐标更新camera相机位置
            */
            if(ResolveCollision(playerBox, obj->GetWorldAABB())){
                camera.Position = glm::vec3((playerBox.min.x + playerBox.max.x) * 0.5f, playerBox.max.y, (playerBox.min.z + playerBox.max.z) * 0.5f);
            }
        }

        // 物理模式：地面碰撞检测
        if (!godMode) {
            playerOnGround = false;
            if (useTerrain) {
                // 地形碰撞：用高度场采样替代AABB
                float terrainHeight = terrain.GetHeight(camera.Position.x, camera.Position.z);
                if (playerBox.min.y < terrainHeight) {
                    float boxHeight = playerBox.max.y - playerBox.min.y;
                    playerBox.min.y = terrainHeight;
                    playerBox.max.y = playerBox.min.y + boxHeight;
                    camera.Position = glm::vec3((playerBox.min.x + playerBox.max.x) * 0.5f, playerBox.max.y, (playerBox.min.z + playerBox.max.z) * 0.5f);
                    playerOnGround = true;
                    playerVelocity.y = 0.0f;
                }
            } else {
                // 平坦地板碰撞：使用AABB
                if (ResolveCollision(playerBox, floorCollider)) {
                    camera.Position = glm::vec3((playerBox.min.x + playerBox.max.x) * 0.5f, playerBox.max.y, (playerBox.min.z + playerBox.max.z) * 0.5f);
                    playerOnGround = true;
                    playerVelocity.y = 0.0f;
                }
            }
        }

        // ==========================================
        // 物理球体更新
        // ==========================================
        if (projectileCooldown > 0.0f)
            projectileCooldown -= deltaTime;

        for (auto& p : projectiles) {
            p.age += deltaTime;

            if (p.state == ProjectileState::STOPPED) continue;

            p.velocity.y -= p.gravity * deltaTime;

            float dragFactor = 1.0f - p.airDrag * deltaTime;
            if (dragFactor < 0.0f) dragFactor = 0.0f;
            p.velocity *= dragFactor;

            if (p.state == ProjectileState::ROLLING) {
                float frictionFactor = 1.0f - p.rollFriction * deltaTime;
                if (frictionFactor < 0.0f) frictionFactor = 0.0f;
                p.velocity.x *= frictionFactor;
                p.velocity.z *= frictionFactor;
            }

            p.position += p.velocity * deltaTime;

            AABB projectileBox = p.GetAABB();
            p.onGround = false;

            Rect2D pRect = { projectileBox.min.x, projectileBox.min.z, projectileBox.max.x, projectileBox.max.z };
            std::vector<GameObject*> nearProj;
            sceneTree.Query(pRect, nearProj);

            for (auto* obj : nearProj) {
                if (!obj->hasCollision) continue;
                CollisionInfo ci = ResolveProjectileCollision(projectileBox, obj->GetWorldAABB());
                if (ci.hit) {
                    float vn = glm::dot(p.velocity, ci.normal);
                    if (vn < 0.0f)
                        p.velocity -= (1.0f + p.restitution) * vn * ci.normal;
                    if (ci.normal.y > 0.5f)
                        p.onGround = true;
                }
            }

            if (useTerrain) {
                float groundY = terrain.GetHeight(p.position.x, p.position.z);
                if (projectileBox.min.y < groundY) {
                    float h = projectileBox.max.y - projectileBox.min.y;
                    projectileBox.min.y = groundY;
                    projectileBox.max.y = projectileBox.min.y + h;
                    glm::vec3 groundNormal = glm::vec3(0.0f, 1.0f, 0.0f);
                    float vn = glm::dot(p.velocity, groundNormal);
                    if (vn < 0.0f)
                        p.velocity -= (1.0f + p.restitution) * vn * groundNormal;
                    p.onGround = true;
                }
            } else {
                CollisionInfo ci = ResolveProjectileCollision(projectileBox, floorCollider);
                if (ci.hit && ci.normal.y > 0.5f) {
                    float vn = glm::dot(p.velocity, ci.normal);
                    if (vn < 0.0f)
                        p.velocity -= (1.0f + p.restitution) * vn * ci.normal;
                    p.onGround = true;
                }
            }

            p.position = (projectileBox.min + projectileBox.max) * 0.5f;

            if (p.onGround && p.state == ProjectileState::BOUNCING &&
                -p.velocity.y < 1.0f && glm::abs(p.velocity.y) < 2.0f) {
                p.state = ProjectileState::ROLLING;
                p.velocity.y = 0.0f;
            }

            if (p.state == ProjectileState::ROLLING) {
                float xzSpeed = glm::length(glm::vec2(p.velocity.x, p.velocity.z));
                if (xzSpeed < 0.3f) {
                    p.state = ProjectileState::STOPPED;
                    p.velocity = glm::vec3(0.0f);
                }
            }

            // if (p.age > p.lifetime) {
            //     p.state = ProjectileState::STOPPED;
            // }
        }
        // 停止就移除(速度接近0或者大于生命周期都停止)
        // projectiles.erase(
        //     std::remove_if(projectiles.begin(), projectiles.end(),
        //         [](const Projectile& p) { return p.state == ProjectileState::STOPPED; }),
        //     projectiles.end());

        // 生命结束才移除
        projectiles.erase(
            std::remove_if(projectiles.begin(), projectiles.end(),
                [](const Projectile& p) { return p.age > p.lifetime; }),
            projectiles.end());


        // 执行关卡切换（四叉树也应该重建一下）
        if (!levelToLoad.empty()) {
            std::cout << "Trigger activated! Loading next level: " << levelToLoad << std::endl;       
            // 卸载当前关卡 (清理内存)
            Level::Instance().Unload(); 
            // 加载新关卡
            Level::Instance().Load(levelToLoad);
            // 传送玩家到新关卡的出生点
            camera.Position = Level::Instance().playerSpawn;
            // 你可以在这里加个渐黑屏幕或者提示音

            sceneTree.Clear();
            for (GameObject* obj : Level::Instance()._objects) {
                sceneTree.Insert(obj);
            }
            SetupForLevel();
        }

        // ==========================================
        // 渲染剔除优化
        // ==========================================
        // 给摄像机定一个可见范围

        float viewDist = 25.0f; 
        Rect2D cameraViewRect = {
            camera.Position.x - viewDist, camera.Position.z - viewDist,
            camera.Position.x + viewDist, camera.Position.z + viewDist
        };

        std::vector<GameObject*> visibleObjects;
        sceneTree.Query(cameraViewRect, visibleObjects);
        // 然后下面所有的object渲染，都使用visibleObjects，而不是sceneObjects

        // ==========================================
        // === 优化：按着色器与模型进行状态排序（批处理 | 串行 此处可并行）===
        // ==========================================
        // std::sort(visibleObjects.begin(), visibleObjects.end(),[](GameObject* a, GameObject* b) {
        //     // 第一优先级：按 Shader 排序 (减少 glUseProgram 调用)
        //     if (a->shaderName != b->shaderName) {
        //         // std::string是可以直接按字典序比较大小的，有重载运算符实现
        //         return a->shaderName < b->shaderName;
        //     }
        //     // 第二优先级：按 模型 排序 (减少 VAO/VBO 切换)
        //     return a->modelName < b->modelName;
        // });

        // 并行排序可见物体
        std::sort(std::execution::par, visibleObjects.begin(), visibleObjects.end(),
            [](GameObject* a, GameObject* b) {
                if (a->shaderName != b->shaderName)
                    return a->shaderName < b->shaderName;
                return a->modelName < b->modelName;
            });
      
        if (needTerrainRegen) {
            terrain.Regenerate();
            needTerrainRegen = false;
        }

        // --- 渲染流程开始 ---
        // 如果灯源旋转
        if (lightRotate){
            float time = static_cast<float>(glfwGetTime());
            float angle = glm::radians(time * 45.0f);  // 45°/秒，转为弧度

            // 直接用三角函数计算圆周运动位置
            lightPos.x = cos(angle) * lightRadius;
            lightPos.z = sin(angle) * lightRadius;
            lightPos.y = lightHeight;  // 保持固定高度
        }

        // 平行光没有实际位置，所以我们根据光源方向反推一个"虚拟位置"来观察场景
        glm::vec3 dirLightTarget = glm::vec3(0.0f); // 场景中心
        // 沿着太阳光反方向退后一段距离（如10.0f），作为相机的虚拟位置
        glm::vec3 dirLightPos = dirLightTarget - glm::normalize(sunDir) * 15.0f;

        
        glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
        if (shadowOn) {
            glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 60.0f);
            glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjection * lightView;
            // 1. 生成阴影贴图
            if (useTerrain) {
                renderer->RenderTerrainShadow(terrain, lightSpaceMatrix);
            }
            renderer->RenderShadowPass(visibleObjects, lightSpaceMatrix);
        }
        // 2. 主场景渲染
        renderer->RenderMainPass(visibleObjects, camera, lightSpaceMatrix, lightPos, sunDir, shadowOn, (float)width, (float)height, frustum, windStrenth, windSpeed);
        
        // 3. 其他环境渲染
        if (useTerrain) {
            renderer->RenderTerrain(terrain, camera, lightSpaceMatrix, lightPos, sunDir, shadowOn, (float)width, (float)height);
            renderer->RenderWater(water, camera, lightSpaceMatrix, lightPos, sunDir, shadowOn, (float)width, (float)height);
        } else {
            renderer->RenderFloor(camera, lightSpaceMatrix, lightPos, sunDir, shadowOn, (float)width, (float)height);
        }
        renderer->RenderLightCube(camera, lightPos, dirLightPos,(float)width, (float)height);
        renderer->RenderSkybox(camera, (float)width, (float)height);

        // 渲染物理球体
        if (!projectiles.empty()) {
            std::vector<glm::mat4> projModels;
            for (const auto& p : projectiles)
                projModels.push_back(p.GetModelMatrix());
            renderer->RenderSpheres(projModels, glm::vec3(1.0f, 0.5f, 0.1f), camera, (float)width, (float)height);
        }

        if (showBox){
            renderer->RenderAABBs(visibleObjects, camera, (float)width, (float)height);
        }

     
        // 4. 渲染悬浮菜单UI
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
        myMenu.Draw(*ResourceManager::GetShader("menu"), camera.GetViewMatrix(), projection);



        // 5. 渲染 ImGui 调试面板
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

 
        terminal.Draw();

        // 渲染地图编辑器
        if (LevelEditor::Instance().isVisible){
            LevelEditor::Instance().RenderUI(g_Game);
        }

        if (ImGui::Begin("TEST")) {
            // ImGui::Text("Frame: %d", frame); // 需在 Game.h 加 int frame = 0; 并每帧++
            ImGui::Separator();
            for (auto* obj : visibleObjects) {
                if (obj->name == "Sk2") {
                    bool inFrustum = frustum.isBoxVisible(obj->GetWorldAABB());
                    ImGui::Text("%s", obj->name.c_str());
                    ImGui::SameLine(); ImGui::TextColored(
                        ImVec4(inFrustum ? 0.0f : 1.0f, 
                               inFrustum ? 1.0f : 0.0f, 0.0f, 1.0f),
                        "GPU: %s", inFrustum ? "ON" : "OFF"
                    );
                    ImGui::Separator();
                    Model* model = ResourceManager::GetModel(obj->modelName);
                    ImGui::Text("LOD: %d", model->currentLOD);
                }
            }
        }
        ImGui::End();
      
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
