#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <fstream> // 文件流
#include <sstream> // 字符串流

#include "World/World.hpp"
#include "Physics/Player.hpp"
#include "Math/Raycast.hpp"
#include "Graphics/Shader.hpp"
#include "Math/Frustum.hpp"

const int SCR_WIDTH = 1280;
const int SCR_HEIGHT = 720;

Player player(glm::vec3(32.0f, 60.0f, 32.0f));
World* globalWorld = nullptr;

float deltaTime = 0.0f, lastFrame = 0.0f;
float lastX = SCR_WIDTH/2.0f, lastY = SCR_HEIGHT/2.0f;
bool firstMouse = true;
bool keys[1024] = {0};

std::string readFile(const char* path) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        file.open(path);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        return stream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::FILE_NOT_SUCCESFULLY_READ: " << path << std::endl;
        return "";
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS && globalWorld) {
        RayHit hit = Raycaster::Cast(*globalWorld, player.camera.Pos, player.camera.Front, 8.0f);
        if (hit.hit) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                globalWorld->setBlock(hit.blockPos.x, hit.blockPos.y, hit.blockPos.z, AIR);
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                glm::ivec3 placePos = hit.blockPos + hit.faceNormal;
                if (glm::distance(glm::vec3(placePos), player.position) > 1.5f) {
                    globalWorld->setBlock(placePos.x, placePos.y, placePos.z, player.selectedBlock);
                }
            }
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) player.toggleMode();
    if (action == GLFW_PRESS) {
        if(key == GLFW_KEY_1) player.selectedBlock = GRASS;
        if(key == GLFW_KEY_2) player.selectedBlock = DIRT;
        if(key == GLFW_KEY_3) player.selectedBlock = STONE;
        if(key == GLFW_KEY_4) player.selectedBlock = SAND;
        if(key == GLFW_KEY_5) player.selectedBlock = WATER;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    player.camera.ProcessMouse(xpos - lastX, lastY - ypos);
    lastX = xpos; lastY = ypos;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "MyCraft Final", NULL, NULL);
    if (!window) { std::cerr << "Failed to create window" << std::endl; return -1; }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glfwSwapInterval(0); // 0 = 关闭 VSync (无限 FPS), 1 = 开启 (锁帧)
    
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::string chunkVS = readFile("res/shaders/chunk.vs");
    std::string chunkFS = readFile("res/shaders/chunk.fs");
    std::string lineVS = readFile("res/shaders/line.vs");
    std::string lineFS = readFile("res/shaders/line.fs");
    std::string uiVS = readFile("res/shaders/ui.vs");
    std::string uiFS = readFile("res/shaders/ui.fs");

    Shader blockShader(chunkVS.c_str(), chunkFS.c_str());
    Shader lineShader(lineVS.c_str(), lineFS.c_str());
    Shader uiShader(uiVS.c_str(), uiFS.c_str());

    Frustum frustum;

    PerlinNoise noise(123);
    World world(noise);
    globalWorld = &world;
    
    int viewDist = 6;
    for(int x=-viewDist; x<=viewDist; ++x) {
        for(int z=-viewDist; z<=viewDist; ++z) world.addChunk(x, z);
    }

    // --- UI 数据 ---
    // 1. 准星 (十字)
    float crosshairVerts[] = { -0.02f, 0.0f, 0.02f, 0.0f, 0.0f, -0.03f, 0.0f, 0.03f };
    GLuint crossVAO, crossVBO;
    glGenVertexArrays(1, &crossVAO); glGenBuffers(1, &crossVBO);
    glBindVertexArray(crossVAO); glBindBuffer(GL_ARRAY_BUFFER, crossVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVerts), crosshairVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0); glEnableVertexAttribArray(0);

    // 2. 方块图标 
    // 坐标 0,0 到 1,1，方便后面用 Model 矩阵缩放位移
    float iconVerts[] = { 
        0.0f, 1.0f,  0.0f, 0.0f,  1.0f, 0.0f, 
        1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f 
    };
    GLuint iconVAO, iconVBO;
    glGenVertexArrays(1, &iconVAO); glGenBuffers(1, &iconVBO);
    glBindVertexArray(iconVAO); glBindBuffer(GL_ARRAY_BUFFER, iconVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(iconVerts), iconVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0); glEnableVertexAttribArray(0);

    // --- 线框数据 ---
    float cubeLines[] = { 
        0,0,0, 1,0,0,  1,0,0, 1,1,0,  1,1,0, 0,1,0,  0,1,0, 0,0,0,
        0,0,1, 1,0,1,  1,0,1, 1,1,1,  1,1,1, 0,1,1,  0,1,1, 0,0,1,
        0,0,0, 0,0,1,  1,0,0, 1,0,1,  1,1,0, 1,1,1,  0,1,0, 0,1,1 
    }; 
    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO); glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO); glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeLines), cubeLines, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0); glEnableVertexAttribArray(0);

    // 定义在 main 函数外或作为静态变量
    char titleBuffer[256]; 

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // --- FPS 计算与标题显示 ---
        static float fpsTimer = 0.0f;
        static int frameCounter = 0;
        fpsTimer += deltaTime;
        frameCounter++;
        if (fpsTimer >= 0.25f) {
            float fps = frameCounter / fpsTimer;
            float ms = 1000.0f / fps;
            sprintf(titleBuffer, "MyCraft - FPS: %.1f (%.2f ms)", fps, ms);
            glfwSetWindowTitle(window, titleBuffer);
            
            fpsTimer = 0.0f;
            frameCounter = 0;
        }


        bool inputs[6] = { keys[GLFW_KEY_W], keys[GLFW_KEY_S], keys[GLFW_KEY_A], keys[GLFW_KEY_D], keys[GLFW_KEY_SPACE], keys[GLFW_KEY_LEFT_CONTROL] };
        player.update(deltaTime, world, inputs);

        glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blockShader.use();
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 500.0f);
        glm::mat4 view = player.camera.GetViewMatrix();
        // 更新视锥体 
        frustum.update(proj * view);

        blockShader.setMat4("projection", proj);
        blockShader.setMat4("view", view);
        for(auto& pair : world.chunks) {
            if (!pair.second) continue; // 空指针检查

            // 视锥体剔除 
            if (!frustum.isBoxVisible(pair.second->aabb)) continue;
            // TODO : 方块是否应该使用实例化绘制？ 等待验证完善
            pair.second->update();
            pair.second->render();
        }

        RayHit hit = Raycaster::Cast(world, player.camera.Pos, player.camera.Front, 8.0f);
        if (hit.hit) {
            lineShader.use();
            lineShader.setMat4("projection", proj);
            lineShader.setMat4("view", view);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(hit.blockPos));
            model = glm::scale(model, glm::vec3(1.002f)); 
            model = glm::translate(model, glm::vec3(-0.001f));
            lineShader.setMat4("model", model);
            glBindVertexArray(lineVAO);
            glDrawArrays(GL_LINES, 0, 24);
        }

        // 3. Render UI
        glDisable(GL_DEPTH_TEST);
        uiShader.use();
        
        // A. 准星 (白色)
        glm::mat4 crossModel = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, (float)SCR_WIDTH/SCR_HEIGHT, 1.0f));
        uiShader.setMat4("model", crossModel);
        uiShader.setMat4("uColor", glm::mat4(1.0f)); 
        glUniform3f(glGetUniformLocation(uiShader.ID, "uColor"), 1.0f, 1.0f, 1.0f);
        
        glBindVertexArray(crossVAO);
        glDrawArrays(GL_LINES, 0, 4);

        // B. 左上角当前方块图标
        glm::vec3 blockColor = Chunk::getColor(player.selectedBlock, 1, true);
        glUniform3f(glGetUniformLocation(uiShader.ID, "uColor"), blockColor.r, blockColor.g, blockColor.b);

        float iconSize = 0.05f;
        float aspectRatio = (float)SCR_WIDTH / SCR_HEIGHT;
        
        glm::mat4 iconModel = glm::mat4(1.0f);
        iconModel = glm::translate(iconModel, glm::vec3(-0.95f, 0.85f, 0.0f)); 
        iconModel = glm::scale(iconModel, glm::vec3(iconSize, iconSize * aspectRatio, 1.0f));
        
        uiShader.setMat4("model", iconModel);
        glBindVertexArray(iconVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}