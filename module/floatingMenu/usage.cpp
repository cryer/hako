#include "GazeMenu.h"

int main() {
    // ... OpenGL初始化, Window创建,等 ...

    Shader menuShader("menu_shader.vs", "menu_shader.fs");

    // 实例化菜单模块 (距离摄像机2.5个单位，向后倾斜20度)
    GazeMenu myMenu(2.5f, 20.0f);
    myMenu.Init();

    // 绘制默认菜单选项
    MenuItem btnStart;
    btnStart.name = "StartGame";
    btnStart.localPosition = glm::vec3(-0.6f, 0.0f, 0.0f); // 放在中心偏左
    btnStart.size = glm::vec2(1.0f, 0.5f);
    btnStart.defaultColor = glm::vec3(0.5f, 0.5f, 0.5f);   // 灰色
    btnStart.highlightColor = glm::vec3(0.2f, 0.8f, 0.2f); // 瞄准时变绿
    btnStart.textureID = 0; // loadTexture("start.png"); 替换为真实贴图
    btnStart.onClick =[]() {
        std::cout << "Start Button Clicked!" << std::endl;
        // 在这里写进入游戏的逻辑
    };
    myMenu.AddItem(btnStart);

    MenuItem btnExit;
    btnExit.name = "ExitGame";
    btnExit.localPosition = glm::vec3(0.6f, 0.0f, 0.0f); // 放在中心偏右
    btnExit.size = glm::vec2(1.0f, 0.5f);
    btnExit.defaultColor = glm::vec3(0.5f, 0.5f, 0.5f);
    btnExit.highlightColor = glm::vec3(0.8f, 0.2f, 0.2f); // 瞄准时变红
    btnExit.onClick =[]() {
        std::cout << "Exit Button Clicked!" << std::endl;
        exit(0);
    };
    myMenu.AddItem(btnExit);

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;

        // --- 输入处理 --- 防抖，防止按一下触发多次
       // 按 'M' 键呼出/隐藏悬浮菜单
        static bool mKeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            if (!mKeyPressed) {
                myMenu.Toggle(camera);
                mKeyPressed = true;
            }
        } else {
            mKeyPressed = false;
        }
      // 鼠标左键 或 E键 触发选中
        static bool eKeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            if (!eKeyPressed) {
                myMenu.Interact();
                eKeyPressed = true;
            }
        } else {
            eKeyPressed = false;
        }


        // --- 逻辑更新 ---
        // 把摄像机传进去计算射线和UI位置
        myMenu.Update(camera, deltaTime);

        // --- 渲染 ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 渲染世界物体...

        // 最后渲染 UI (确保在最上层)【注意projection view 重命名问题】
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        myMenu.Draw(menuShader, view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}