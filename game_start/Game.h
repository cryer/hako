#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <functional>
#include <unordered_map>
#include "camera.h"
#include "Renderer.h"
#include "GameObject.h"
#include "PlayerWeapon.h"
#include "DebugTerminal.h"
#include "GazeMenu.h"
#include "ResourceManager.h"
#include "AABB.h"

class Game {
public:
    int width, height;
    GLFWwindow* window;
    
    Camera camera;
    Renderer* renderer;
    DebugTerminal terminal;
    GazeMenu myMenu;

    std::vector<GameObject*> sceneObjects;
    std::vector<AABB> aabbs;

    AABB playerBox;

    // 游戏全局状态
    glm::vec3 lightPos = glm::vec3(1.2f, 3.0f, 3.0f);
    glm::vec3 sunDir = glm::vec3(-0.2f, -1.0f, -0.3f);
    // 旋转光源使用
    bool lightRotate = false;
    const float lightRadius = 2.5f;  // 公转半径
    const float lightHeight = 3.0f;  // Y轴高度（可选）

    bool showGun = false;
    bool shadowOn = false;
    bool aabbBox = true;
    
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float lastX, lastY;
    bool firstMouse = true;
    bool lastTerminalVisible = false;

    Game(int w, int h);
    ~Game();

    bool Init(const char* title);
    void Run();

    void ProcessInput();
    void SetupMenu();
    void AddObject(GameObject* obj) { sceneObjects.push_back(obj); }
    void AddAABB(GameObject* obj, Model* model) {
        glm::mat4 tansformed = obj->transform.GetMatrix();
        AABB newAABB = model->localAABB.GetTransformed(tansformed);
        aabbs.push_back(newAABB);
    }
private:
    // 辅助函数：创建并返回一个配置好的 MenuItem
    MenuItem CreateMenuItem(
        const std::string& name,
        const glm::vec3& pos,
        const glm::vec2& size,
        const std::string& textureName,
        std::function<void()> onClickCallback,
        float zDepth = 0.0f // 默认Z轴，背景板可以传-0.02f
    );
};