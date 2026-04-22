#include "GazeMenu.h"
#include <iostream>

GazeMenu::GazeMenu(float distance, float tilt) 
    : isActive(false), distanceToCamera(distance), tiltAngle(tilt), VAO(0), VBO(0) {
}

GazeMenu::~GazeMenu() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
}

void GazeMenu::Init() {
    // 基础的正方形Quad，中心在(0,0,0)
    float vertices[] = {
        // positions          // texture coords
         0.5f, -0.5f, 0.0f,    1.0f, 0.0f,   // bottom right
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   // top right 
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // top left 
    };
    unsigned int indices[] = {
        0, 1, 2, // first triangle
        1, 3, 2  // second triangle
    };

    GLuint EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void GazeMenu::AddItem(const MenuItem& item) {
    items.push_back(item);
}

void GazeMenu::Toggle(const Camera& camera) {
    isActive = !isActive;
    if (isActive) {
        // 呼出时，锁定当前的Yaw（偏航角），这样玩家回头时菜单不会跟着转，方便视线选择
        currentYaw = camera.Yaw; 
    }
}

void GazeMenu::Update(const Camera& camera, float deltaTime) {
    if (!isActive) return;

    // 1. 计算菜单的世界坐标与姿态
    // 菜单始终在玩家"位置"的前方，但朝向是我们锁定的 Yaw，这样人物走动菜单跟着动，但头转动时菜单不转。
    glm::vec3 lockedFront;
    lockedFront.x = cos(glm::radians(currentYaw)) * cos(glm::radians(0.0f));
    lockedFront.y = 0.0f; // 保持水平
    lockedFront.z = sin(glm::radians(currentYaw)) * cos(glm::radians(0.0f));
    lockedFront = glm::normalize(lockedFront);

    menuWorldPos = camera.Position + lockedFront * distanceToCamera;
    
    // 计算菜单的局部坐标轴 (带倾斜角)
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, glm::radians(-currentYaw - 90.0f), glm::vec3(0, 1, 0)); // 面对摄像机
    rotation = glm::rotate(rotation, glm::radians(tiltAngle), glm::vec3(1, 0, 0)); // 向后倾斜

    menuRight   = glm::vec3(rotation[0]);
    menuUp      = glm::vec3(rotation[1]);
    menuForward = glm::vec3(rotation[2]); // 菜单面板的法线方向

    // 2. 射线检测 (Gaze Selection)
    glm::vec3 rayOrigin = camera.Position;
    glm::vec3 rayDir = camera.Front; // 摄像机的中心视角视线

    for (auto& item : items) {
        // 计算每个Item在世界空间中的真实位置
        glm::vec3 itemWorldPos = menuWorldPos 
                               + menuRight * item.localPosition.x 
                               + menuUp * item.localPosition.y 
                               + menuForward * item.localPosition.z;

        item.isHovered = RayIntersectsQuad(rayOrigin, rayDir, itemWorldPos, menuForward, menuUp, menuRight, item.size);
    }
}

// 射线与具有大小和方向的矩形面板进行相交检测
bool GazeMenu::RayIntersectsQuad(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::vec3 quadCenter, glm::vec3 quadNormal, glm::vec3 quadUp, glm::vec3 quadRight, glm::vec2 size) {
    // 射线与平面的交点计算 t = dot(Center - Origin, Normal) / dot(Dir, Normal)
    float denom = glm::dot(rayDir, quadNormal);
    if (abs(denom) > 1e-6) {
        float t = glm::dot(quadCenter - rayOrigin, quadNormal) / denom;
        if (t >= 0) { // 交点在前方
            glm::vec3 p = rayOrigin + rayDir * t;
            // 判断交点 P 是否在四边形范围内
            glm::vec3 d = p - quadCenter;
            float distRight = glm::dot(d, quadRight);
            float distUp = glm::dot(d, quadUp);

            if (abs(distRight) <= size.x / 2.0f && abs(distUp) <= size.y / 2.0f) {
                return true;
            }
        }
    }
    return false;
}

void GazeMenu::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    if (!isActive) return;

    shader.use();
    shader.setMatrix4fv("view", view);
    shader.setMatrix4fv("projection", projection);

    // 禁用深度写入或开启混合，让菜单看起来像HUD
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glDisable(GL_DEPTH_TEST); // 如果想让菜单穿透墙壁显示可以解开这行

    glBindVertexArray(VAO);

    for (const auto& item : items) {
        glm::mat4 model = glm::mat4(1.0f);
        
        // 平移到Item位置
        glm::vec3 itemWorldPos = menuWorldPos 
                               + menuRight * item.localPosition.x 
                               + menuUp * item.localPosition.y 
                               + menuForward * item.localPosition.z;
        model = glm::translate(model, itemWorldPos);
        
        // 旋转匹配菜单面板 (利用构建好的正交基)
        glm::mat4 rotMat(1.0f);
        rotMat[0] = glm::vec4(menuRight, 0.0f);
        rotMat[1] = glm::vec4(menuUp, 0.0f);
        rotMat[2] = glm::vec4(menuForward, 0.0f);
        model *= rotMat;

        // 缩放
        model = glm::scale(model, glm::vec3(item.size.x, item.size.y, 1.0f));
        shader.setMatrix4fv("model", model);

        // 设置颜色和高亮状态
        glm::vec3 renderColor = item.isHovered ? item.highlightColor : item.defaultColor;
        shader.setFloat3("color", renderColor);
        
        // 处理贴图 (假设你的Shader有 "useTexture" 和 "image" 变量)
        GLuint tex = item.isHovered && item.highlightTexID != 0 ? item.highlightTexID : item.textureID;
        if (tex != 0) {
            shader.setInt("useTexture", 1);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex);
            shader.setInt("image", 0);
        } else {
            shader.setInt("useTexture", 0);
        }

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    // glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void GazeMenu::Interact() {
    if (!isActive) return;
    for (auto& item : items) {
        if (item.isHovered && item.onClick) {
            item.onClick(); // 触发绑定的回调函数
            // 如果点击后希望关闭菜单，可以在这里加 Toggle(); 或者在外部处理
            break; // 一次只触发一个
        }
    }
}