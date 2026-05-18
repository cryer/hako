#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <functional>
#include "camera.h"
#include "Renderer.h"
#include "GameObject.h"
#include "DebugTerminal.h"
#include "GazeMenu.h"
#include "AABB.h"
#include "Frustum.h"
#include "LevelEditor.h"
#include "QuadTree.h"
#include "FpsLimiter.h"
#include "AudioManager.h"
#include "Terrain.h"

class Game {
public:
    int width, height;
    GLFWwindow* window;

    Camera camera;
    DebugTerminal terminal;
    GazeMenu myMenu;
    AABB playerBox;
    Frustum frustum;
    QuadTree sceneTree;
    Terrain terrain;

    FrameLimiter limiter;
    AudioManager audio;


    // Renderer类因为构造需要初始化VAO VBO 顶点属性，以及绑定设置纹理这些
    // 因此需要创建了GLAW上下文之后才能使用，所以需要延迟构造，因此使用指针
    // 因为生命周期和Game一致，且所有权是Game类管理，因此使用智能指针
    std::unique_ptr<Renderer> renderer;

    
    // 游戏全局状态
    glm::vec3 lightPos = glm::vec3(1.2f, 3.0f, 3.0f);
    glm::vec3 sunDir = glm::vec3(-0.2f, -1.0f, -0.3f);
    // 旋转光源使用
    bool lightRotate = false;
    const float lightRadius = 2.5f;  // 公转半径
    const float lightHeight = 3.0f;  // Y轴高度


    bool showGun = false;
    bool shadowOn = false;
    bool showBox = false;
    
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float lastX, lastY;
    bool firstMouse = true;
    bool lastTerminalVisible = false;
    bool lastEditorVisible = false;
    bool isBgmPause = false;
    bool needTerrainRegen = false;
    bool useTerrain = false;

    Game(int w, int h);
    ~Game();

    bool Init(const char* title);
    void Run();
    void SetupForLevel();

    void ProcessInput();
    void SetupMenu();

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
