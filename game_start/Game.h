#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "camera.h"
#include "Renderer.h"
#include "GameObject.h"
#include "PlayerWeapon.h"
#include "DebugTerminal.h"
#include "GazeMenu.h"

class Game {
public:
    int width, height;
    GLFWwindow* window;
    
    Camera camera;
    Renderer* renderer;
    DebugTerminal terminal;
    GazeMenu myMenu;
    
    std::vector<GameObject*> sceneObjects;

    // 游戏全局状态
    glm::vec3 lightPos = glm::vec3(1.2f, 3.0f, 3.0f);
    bool showGun = false;
    bool shadowOn = false;
    
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float lastX, lastY;
    bool firstMouse = true;
    bool lastTerminalVisible = false;

    Game(int w, int h, const char* title);
    ~Game();

    bool Init(const char* title);
    void Run();

    void ProcessInput();
    void SetupMenu();
    void AddObject(GameObject* obj) { sceneObjects.push_back(obj); }
};