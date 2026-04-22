#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <functional>
#include "camera.h" // 假设你的摄像机头文件
#include "shader.h" // 假设你的着色器头文件

// 菜单单项结构体
struct MenuItem {
    std::string name;
    glm::vec3 localPosition; // 相对于菜单中心的偏移量
    glm::vec2 size;          // 宽高
    GLuint textureID = 0;        // 默认贴图
    GLuint highlightTexID = 0;   // 高亮贴图
    glm::vec3 defaultColor;  // 如果没有贴图时的默认颜色
    glm::vec3 highlightColor;// 高亮颜色
    
    bool isHovered = false;
    std::function<void()> onClick; // 触发时的回调函数
};

class GazeMenu {
public:
    GazeMenu(float distance = 2.0f, float tiltAngle = 15.0f);
    ~GazeMenu();

    // 初始化渲染数据 (VAO/VBO)
    void Init();

    // 添加菜单选项
    void AddItem(const MenuItem& item);

    // 核心逻辑更新（处理射线检测、跟随等）
    void Update(const Camera& camera, float deltaTime);

    // 绘制菜单
    void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection);

    // 呼出/关闭菜单
    void Toggle(const Camera& camera);
    bool IsActive() const { return isActive; }

    // 当按下确认键时调用（触发中心点选中的选项）
    void Interact();

private:
    bool isActive;
    float distanceToCamera; // 菜单离摄像机的距离
    float tiltAngle;        // 菜单倾斜角度(度数)，向后仰

    glm::vec3 menuWorldPos;
    glm::vec3 menuForward;  // 菜单的法线方向
    glm::vec3 menuUp;
    glm::vec3 menuRight;
    float currentYaw;       // 菜单锁定的偏航角

    std::vector<MenuItem> items;

    // OpenGL 渲染数据
    GLuint VAO, VBO;

    // 射线与四边形相交检测
    bool RayIntersectsQuad(glm::vec3 rayOrigin, glm::vec3 rayDir, 
                           glm::vec3 quadCenter, glm::vec3 quadNormal, 
                           glm::vec3 quadUp, glm::vec3 quadRight, glm::vec2 size);
};