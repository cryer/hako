#include <windows.h>
#include "Game.h"
#include "ResourceManager.h"
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <windows.h>


// 全局静态指针，用于回调函数访问
static Game* g_Game = nullptr; 

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    if (g_Game) { g_Game->width = width; g_Game->height = height; }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (!g_Game || g_Game->terminal.GetVisible()) return;
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
    if (!g_Game || g_Game->terminal.GetVisible()) return;
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

Game::Game(int w, int h) : width(w), height(h), camera(glm::vec3(0.0f, 3.0f, 3.0f)), myMenu(1.5f, -10.0f) {
    g_Game = this;
    lastX = w / 2.0f;
    lastY = h / 2.0f;
}

Game::~Game() {
    ResourceManager::Clear();
    for (auto obj : sceneObjects) delete obj;
    delete renderer;
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
        {"ExitGame", {0.55f, 0.35f, 0.f}, {0.2f, 0.2f}, "exit",   [](){ std::cout << "Exit\n"; exit(0); }},
        {"YouTube",  {-0.55f, 0.35f, 0.f},{0.25f, 0.2f},"youtube",
                    [](){
                          OpenChromeBrowser(
                            L"https://www.youtube.com/");
                          std::cout << "YouTube\n"; 
                        }
        },
        {"VS",       {-0.55f, 0.15f, 0.f},{0.25f, 0.2f},"vs",     [](){ std::cout << "VS\n"; }},
        {"Clash",    {-0.55f, -0.05f, 0.f},{0.25f, 0.2f},"clash", [](){ std::cout << "Clash\n"; }},
        
        // 下面两个需要捕获 this，所以 lambda 写法略有不同，但在 std::function 中是兼容的
        {"LD", {-0.55f, -0.25f, 0.f},{0.25f, 0.2f},"ld", 
                        [this](){ lightRotate = !lightRotate; 
                        std::cout << "LD\n"; 
                                }
        },
        {"SS", {-0.55f, -0.45f, 0.f},{0.25f, 0.2f},"ss", 
                        [this](){ 
                                shadowOn = !shadowOn; 
                                std::cout << "SS\n"; 
                                }
        },
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
    terminal.BindFloat("lightPosX", &lightPos.x);
    terminal.BindFloat("lightPosY", &lightPos.y);
    terminal.BindFloat("lightPosZ", &lightPos.z);
    terminal.BindBool("showGun", &showGun);
    terminal.BindBool("shadowOn", &shadowOn);
    terminal.BindBool("lightRotate", &lightRotate);
    terminal.BindBool("aabbBox", &aabbBox);

    terminal.RegisterCommand("resetLight", [&](const std::vector<std::string>& args) {
        lightPos = glm::vec3(1.2f, 3.0f, 3.0f);
        terminal.AddLog("reset light pos.");
    }, "Reset light to default");
    terminal.AddLog("Developer Terminal Ready. Press '~' to toggle.");

    // 初始化渲染器
    renderer = new Renderer();
    
    return true;
}

void Game::ProcessInput() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);

    // 触发武器开火
    if (showGun && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        for (auto obj : sceneObjects) {
            // 通过动态转换找到武器实体，并调用它的专属方法
            // 值得研究 #TODO
            PlayerWeapon* weapon = dynamic_cast<PlayerWeapon*>(obj);
            if (weapon) {
                weapon->Fire(static_cast<float>(glfwGetTime()));
            }
        }
    }
}

void Game::Run() {
    double fpsLastTime = glfwGetTime();
    int frameCount = 0;

    static bool mKeyPressed = false;
    static bool eKeyPressed = false;


    while (!glfwWindowShouldClose(window)) {
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

        // 处理终端导致的鼠标状态变更
        bool currentTerminalVisible = terminal.GetVisible();
        if (currentTerminalVisible != lastTerminalVisible) {
            if (currentTerminalVisible) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true; 
            }
            lastTerminalVisible = currentTerminalVisible;
        }

        // --- GazeMenu 输入与逻辑 ---
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            if (!mKeyPressed) { myMenu.Toggle(camera); mKeyPressed = true; }
        } else mKeyPressed = false;

        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            if (!eKeyPressed) { myMenu.Interact(); eKeyPressed = true; }
        } else eKeyPressed = false;

        myMenu.Update(camera, deltaTime);

        // --- 玩家输入 ---
        if (!currentTerminalVisible) {
            ProcessInput();
        }

        // --- 游戏逻辑更新 (解耦重点：实体自己管自己的运算) ---
        for (auto obj : sceneObjects) {
            // 同步全局 showGun 状态给具体武器
            if (obj->name == "M416") {
                obj->isVisible = showGun;
            }
            
            // 只有当物体激活/可见时，才执行它的逻辑更新
            if (obj->isVisible) {
                obj->Update(deltaTime);
            }
        }

        // 玩家碰撞box 直接每帧创建一个对象就好 没什么性能消耗
        playerBox = AABB::CreateFromCenterAndSize(camera.Position, glm::vec3(2.0f,2.0f,2.0f));
        // cout << "Player AABB Min: (" << playerBox.min.x << ", " << playerBox.min.y << ", " << playerBox.min.z << ")" << endl;

        for (const auto& aabb : aabbs){
            /*
            ResolveCollision内部碰撞会反推第一个参数的box坐标
            利用反推box坐标更新camera相机位置
            */
            if(ResolveCollision(playerBox, aabb)){
                camera.Position = (playerBox.min + playerBox.max) * 0.5f;
            }
            // if (playerBox.Intersects(aabb)){
            //     // cout << "collision! \n";
            // }
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
            glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f);
            glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjection * lightView;
            // 1. 生成阴影贴图
            renderer->RenderShadowPass(sceneObjects, lightSpaceMatrix);
        }

        // 2. 主场景渲染
        renderer->RenderMainPass(sceneObjects, camera, lightSpaceMatrix, lightPos, sunDir, shadowOn, (float)width, (float)height);
        
        // 3. 其他环境渲染
        renderer->RenderFloor(camera, lightSpaceMatrix, lightPos, shadowOn, (float)width, (float)height);
        renderer->RenderLightCube(camera, lightPos, dirLightPos,(float)width, (float)height);
        renderer->RenderSkybox(camera, (float)width, (float)height);
        if (aabbBox){
            renderer->RenderAABBs(aabbs, camera, (float)width, (float)height);
        }

     
        // 4. 渲染悬浮菜单UI
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
        myMenu.Draw(*ResourceManager::GetShader("menu"), camera.GetViewMatrix(), projection);

        // 5. 渲染 ImGui 调试面板
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        terminal.Draw();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}