#include "Game.h"
#include "ResourceManager.h"
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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

void Game::SetupMenu() {
    myMenu.Init();
    MenuItem bgBoard;
    bgBoard.name = "BackgroundBoard";
    bgBoard.localPosition = glm::vec3(0.0f, 0.0f, -0.02f); 
    bgBoard.size = glm::vec2(1.4f, 1.0f); 
    bgBoard.defaultColor = glm::vec3(0.15f, 0.15f, 0.18f); 
    bgBoard.highlightColor = glm::vec3(0.15f, 0.15f, 0.18f); 
    myMenu.AddItem(bgBoard);

    MenuItem btnExit;
    btnExit.name = "ExitGame";
    btnExit.localPosition = glm::vec3(0.5f, 0.3f, 0.0f); 
    btnExit.size = glm::vec2(0.2f, 0.2f);
    btnExit.defaultColor = glm::vec3(1.0f, 1.0f, 1.0f);
    btnExit.highlightColor = glm::vec3(0.2f, 0.8f, 0.2f); 
    btnExit.textureID = ResourceManager::GetTexture("exit");
    btnExit.highlightTexID = ResourceManager::GetTexture("exit");
    btnExit.onClick =[]() {
        std::cout << "Exit Button Clicked!" << std::endl;
        exit(0);
    };
    myMenu.AddItem(btnExit);

    MenuItem btnStart;
    btnStart.name = "StartGame";
    btnStart.localPosition = glm::vec3(-0.5f, 0.3f, 0.0f); 
    btnStart.size = glm::vec2(0.3f, 0.2f);
    btnStart.defaultColor = glm::vec3(1.0f, 1.0f, 1.0f);
    btnStart.highlightColor = glm::vec3(0.2f, 0.8f, 0.2f);
    btnStart.textureID = ResourceManager::GetTexture("youtube");
    btnStart.highlightTexID = ResourceManager::GetTexture("youtube");
    btnStart.onClick =[]() {
        std::cout << "Start Button Clicked!" << std::endl;
    };
    myMenu.AddItem(btnStart);

    MenuItem btnGit;
    btnGit.name = "GitHub";
    btnGit.localPosition = glm::vec3(0.0f, -0.2f, 0.0f); 
    btnGit.size = glm::vec2(0.9f, 0.6f);
    btnGit.defaultColor = glm::vec3(1.0f, 1.0f, 1.0f);
    btnGit.highlightColor = glm::vec3(0.2f, 0.8f, 0.2f); 
    btnGit.textureID = ResourceManager::GetTexture("github");
    btnGit.highlightTexID = ResourceManager::GetTexture("github");
    btnGit.onClick =[]() {
        std::cout << "Git Button Clicked!" << std::endl;
    };
    myMenu.AddItem(btnGit);
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

        // --- 渲染流程开始 ---
        glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
        if (shadowOn) {
            glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f);
            glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjection * lightView;
            // 1. 生成阴影贴图
            renderer->RenderShadowPass(sceneObjects, lightSpaceMatrix);
        }

        // 2. 主场景渲染
        renderer->RenderMainPass(sceneObjects, camera, lightSpaceMatrix, lightPos, shadowOn, (float)width, (float)height);
        
        // 3. 其他环境渲染
        renderer->RenderFloor(camera, lightSpaceMatrix, lightPos, shadowOn, (float)width, (float)height);
        renderer->RenderLightCube(camera, lightPos, (float)width, (float)height);
        renderer->RenderSkybox(camera, (float)width, (float)height);

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
