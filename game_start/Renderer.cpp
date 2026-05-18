#include "Renderer.h"
#include "mesh.h"

#include <GLFW/glfw3.h>
#include <unordered_map>

Renderer::Renderer() {
    InitPrimitives();
    InitShadowMap();
}

void Renderer::InitShadowMap() {
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::InitPrimitives() {
    // 省略原来的大段数组声明，此处用原数组直接填充
    float skyboxVertices[] = { -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f };
    float planeVertices[] = { 30.0f, -0.5f,  30.0f,  0.0f, 1.0f, 0.0f,  30.0f,  0.0f, -30.0f, -0.5f, -30.0f,  0.0f, 1.0f, 0.0f,   0.0f, 30.0f, -30.0f, -0.5f,  30.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f, 30.0f, -0.5f,  30.0f,  0.0f, 1.0f, 0.0f,  30.0f,  0.0f, 30.0f, -0.5f, -30.0f,  0.0f, 1.0f, 0.0f,  30.0f, 30.0f, -30.0f, -0.5f, -30.0f,  0.0f, 1.0f, 0.0f,  0.0f, 30.0f };
    float vertices[] = { 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };

    // skybox VAO
    glGenVertexArrays(1, &skyboxVAO); glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO); glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Plane VAO
    glGenVertexArrays(1, &planeVAO); glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO); glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    // Cube VAO
    glGenVertexArrays(1, &lightCubeVAO); glGenBuffers(1, &VBO);
    glBindVertexArray(lightCubeVAO); glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Renderer::RenderShadowPass(
            const std::vector<GameObject*>& objects, 
            const glm::mat4& lightSpaceMatrix) {
    Shader* depthShader = ResourceManager::GetShader("depth");
    depthShader->use();
    depthShader->setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT); 

    for (auto obj : objects) {
        if (!obj->isVisible) continue;
        if (obj->useInstancing) continue; // 使用实例化渲染的对象跳过普通的Draw
        if(obj->name != "M416")
            obj->Draw(depthShader);
    }

    Shader* instancedDepthShader = ResourceManager::GetShader("instanced_depth");
    if (instancedDepthShader) {
        instancedDepthShader->use();
        instancedDepthShader->setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);

        std::unordered_map<std::string, std::vector<glm::mat4>> instancedGroups;
        for (auto obj : objects) {
            if (!obj->isVisible) continue;
            if (!obj->useInstancing || obj->instances.empty()) continue;
            if (!obj->castShadow) continue;
            auto& group = instancedGroups[obj->modelName];
            group.insert(group.end(), obj->instances.begin(), obj->instances.end());
        }

        for (auto& [modelName, matrices] : instancedGroups) {
            Model* model = ResourceManager::GetModel(modelName);
            if (!model) continue;
            model->DrawInstanced(*instancedDepthShader, matrices, 0);
        }
    }

    glCullFace(GL_BACK); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderMainPass(
        const std::vector<GameObject*>& objects, 
        Camera& camera, 
        glm::mat4 lightSpaceMatrix,
        glm::vec3 lightPos,
        glm::vec3 sunDir, 
        bool shadowOn, 
        float screenWidth, 
        float screenHeight,
        Frustum& frustum) {
    glViewport(0, 0, screenWidth, screenHeight);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shader* shader = ResourceManager::GetShader("standard");
    shader->use();
    shader->setFloat3("dirLight.direction", sunDir);
    shader->setFloat3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shader->setFloat3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader->setFloat3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    shader->setFloat3("light.position", lightPos);
    shader->setFloat3("light.ambient",  0.2f, 0.2f, 0.2f);
    shader->setFloat3("light.diffuse",  0.5f, 0.5f, 0.5f);
    shader->setFloat3("light.specular", 1.0f, 1.0f, 1.0f);
    shader->setFloat("light.constant", 1.0f);
    shader->setFloat("light.linear", 0.09f);
    shader->setFloat("light.quadratic", 0.032f);
    shader->setFloat3("viewPos", camera.Position);
    shader->setFloat("shininess", 32.0f);

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    shader->setMatrix4fv("projection", projection);
    shader->setMatrix4fv("view", view);
    shader->setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);
    shader->setBool("shadowOn", shadowOn);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    shader->setInt("shadowMap", 10);

    frustum.update(projection * view);
    // 主要场景模型做视锥体剔除，其他地板之类的渲染可以忽略
    for (auto obj : objects) {
        // 如果不可见，跳过主场景渲染
        if (!obj->isVisible) continue;
        // 实例化对象跳过单独绘制，统一在下个pass批处理
        if (obj->useInstancing) continue;
        // 视锥体剔除(只剔除渲染部分，update和碰撞检测保持计算
        // 否则不在视野中的实体就不更新逻辑了以及倒着走
        // 就能无视碰撞。主要节省大量的drawcall)
        AABB worldAABB = obj->GetWorldAABB();
        if (!frustum.isBoxVisible(worldAABB)) continue;

        // 计算模型中心到相机的距离，以此划分 LOD 级别
        glm::vec3 center = (worldAABB.min + worldAABB.max) * 0.5f;
        float distance = glm::distance(camera.Position, center);

        int lodLevel = 0; // 默认 LOD0
        if (distance > 20.0f) {
            lodLevel = 2; // 距离大于 35，切为 25% (LOD2)
        } else if (distance > 10.0f) {
            lodLevel = 1; // 距离在 15-35 之间，切为 50% (LOD1)
        }

        // 传入 lodLevel。
        obj->Draw(shader, lodLevel); 
    }

    // ======== 实例化渲染批处理 ========
    Shader* instancedShader = ResourceManager::GetShader("instanced_standard");
    if (instancedShader) {
        instancedShader->use();
        instancedShader->setFloat3("dirLight.direction", sunDir);
        instancedShader->setFloat3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        instancedShader->setFloat3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        instancedShader->setFloat3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        instancedShader->setFloat3("light.position", lightPos);
        instancedShader->setFloat3("light.ambient",  0.2f, 0.2f, 0.2f);
        instancedShader->setFloat3("light.diffuse",  0.5f, 0.5f, 0.5f);
        instancedShader->setFloat3("light.specular", 1.0f, 1.0f, 1.0f);
        instancedShader->setFloat("light.constant", 1.0f);
        instancedShader->setFloat("light.linear", 0.09f);
        instancedShader->setFloat("light.quadratic", 0.032f);
        instancedShader->setFloat3("viewPos", camera.Position);
        instancedShader->setFloat("shininess", 32.0f);
        instancedShader->setMatrix4fv("projection", projection);
        instancedShader->setMatrix4fv("view", view);
        instancedShader->setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);
        instancedShader->setBool("shadowOn", shadowOn);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        instancedShader->setInt("shadowMap", 10);

        std::unordered_map<std::string, std::vector<glm::mat4>> instancedGroups;
        for (auto obj : objects) {
            if (!obj->isVisible) continue;
            if (!obj->useInstancing || obj->instances.empty()) continue;

            AABB worldAABB = obj->GetWorldAABB();
            if (!frustum.isBoxVisible(worldAABB)) continue;

            auto& group = instancedGroups[obj->modelName];
            group.insert(group.end(), obj->instances.begin(), obj->instances.end());
        }

        for (auto& [modelName, matrices] : instancedGroups) {
            Model* model = ResourceManager::GetModel(modelName);
            if (!model) continue;
            model->DrawInstanced(*instancedShader, matrices, 0);
        }
    }
}

void Renderer::RenderFloor(Camera& camera, glm::mat4 lightSpaceMatrix, glm::vec3 lightPos, bool shadowOn, float screenWidth, float screenHeight) {
    Shader* floorShader = ResourceManager::GetShader("floor");
    floorShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    floorShader->setMatrix4fv("projection", projection);
    floorShader->setMatrix4fv("view", camera.GetViewMatrix());
    floorShader->setFloat3("viewPos", camera.Position);
    floorShader->setFloat3("lightPos", lightPos);
    floorShader->setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);
    floorShader->setBool("shadowOn", shadowOn);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    floorShader->setInt("shadowMap", 10);

    glBindVertexArray(planeVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ResourceManager::GetTexture("wood"));
    floorShader->setInt("floorTexture", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer::RenderSkybox(Camera& camera, float screenWidth, float screenHeight) {
    glDepthFunc(GL_LEQUAL);
    Shader* skyboxShader = ResourceManager::GetShader("skybox");
    skyboxShader->use();
    glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); 
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    skyboxShader->setMatrix4fv("view", view);
    skyboxShader->setMatrix4fv("projection", projection);

    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ResourceManager::GetTexture("skybox"));
    skyboxShader->setInt("skybox", 0);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS); 
}

void Renderer::RenderLightCube(Camera& camera, glm::vec3 lightPos, glm::vec3 sunPos, float screenWidth, float screenHeight) {
    Shader* lightShader = ResourceManager::GetShader("light");
    lightShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    lightShader->setMatrix4fv("view", camera.GetViewMatrix());
    lightShader->setMatrix4fv("projection", projection);
    

    glm::mat4 model = glm::translate(glm::mat4(1.0f), lightPos);
    
    model = glm::scale(model, glm::vec3(0.2f));
    lightShader->setMatrix4fv("model", model);

    glBindVertexArray(lightCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    // 绘制太阳
    model = glm::translate(glm::mat4(1.0f), sunPos);
    lightShader->setMatrix4fv("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Renderer::RenderAABBs(const std::vector<GameObject*>& objects, 
                Camera& camera, 
                float screenWidth, 
                float screenHeight){
    Shader* boxShader = ResourceManager::GetShader("light");
    boxShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    boxShader->setMatrix4fv("view", camera.GetViewMatrix());
    boxShader->setMatrix4fv("projection", projection);

    // 开启线框绘制模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // 开启深度测试，避免线框交叉时显示错误
    // glEnable(GL_DEPTH_TEST); 
    glBindVertexArray(lightCubeVAO);
    for (auto obj : objects){
        AABB aabb = obj->GetWorldAABB();
        // 计算中心点 (平移量)
        glm::vec3 center = (aabb.min + aabb.max) * 0.5f;
        // 计算三轴长度 (缩放量)
        glm::vec3 extent = aabb.max - aabb.min;
        // 构建 Model 矩阵：先缩放，后平移
        glm::mat4 model = glm::translate(glm::mat4(1.0f), center);
        model = glm::scale(model, extent);
        
        boxShader->setMatrix4fv("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 关闭线框绘制模式（即设置填充模式）
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

