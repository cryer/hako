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
    InitSphere();
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

void Renderer::InitSphere() {
    const int stacks = 14;
    const int sectors = 18;
    const float PI = 3.14159265359f;

    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;

    for (int i = 0; i <= stacks; i++) {
        float theta = PI * (float)i / stacks;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        for (int j = 0; j < sectors; j++) {
            float phi = 2.0f * PI * (float)j / sectors;
            float x = sinTheta * cos(phi);
            float y = cosTheta;
            float z = sinTheta * sin(phi);
            sphereVertices.push_back(x);
            sphereVertices.push_back(y);
            sphereVertices.push_back(z);
        }
    }

    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < sectors; j++) {
            unsigned int k1 = i * sectors + j;
            unsigned int k2 = i * sectors + (j + 1) % sectors;
            unsigned int k3 = (i + 1) * sectors + j;
            unsigned int k4 = (i + 1) * sectors + (j + 1) % sectors;

            sphereIndices.push_back(k1);
            sphereIndices.push_back(k3);
            sphereIndices.push_back(k2);

            sphereIndices.push_back(k2);
            sphereIndices.push_back(k3);
            sphereIndices.push_back(k4);
        }
    }

    sphereIndexCount = (unsigned int)sphereIndices.size();

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
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
            // 阴影没必要分LOD，直接用中间的LOD1近似即可
            model->DrawInstanced(*instancedDepthShader, matrices, 1);
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
        Frustum& frustum,
        float windStrenth,
        float windSpeed) {
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
    Shader* grassShader = ResourceManager::GetShader("grass");

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

        // 草地着色器：设置相同的灯光/投影 uniform，外加风力参数
        if (grassShader) {
            grassShader->use();
            grassShader->setFloat3("dirLight.direction", sunDir);
            grassShader->setFloat3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
            grassShader->setFloat3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
            grassShader->setFloat3("dirLight.specular", 0.5f, 0.5f, 0.5f);
            grassShader->setFloat3("light.position", lightPos);
            grassShader->setFloat3("light.ambient",  0.2f, 0.2f, 0.2f);
            grassShader->setFloat3("light.diffuse",  0.5f, 0.5f, 0.5f);
            grassShader->setFloat3("light.specular", 1.0f, 1.0f, 1.0f);
            grassShader->setFloat("light.constant", 1.0f);
            grassShader->setFloat("light.linear", 0.09f);
            grassShader->setFloat("light.quadratic", 0.032f);
            grassShader->setFloat3("viewPos", camera.Position);
            grassShader->setFloat("shininess", 32.0f);
            grassShader->setMatrix4fv("projection", projection);
            grassShader->setMatrix4fv("view", view);
            grassShader->setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);
            grassShader->setBool("shadowOn", shadowOn);
            grassShader->setInt("shadowMap", 10);

            float curTime = (float)glfwGetTime();
            grassShader->setFloat("time", curTime);
            grassShader->setFloat3("windDirection", 0.6f, 0.0f, 0.4f);
            grassShader->setFloat("windStrength", windStrenth);
            grassShader->setFloat("windSpeed", windSpeed);
        }

        instancedShader->use(); // 切回默认实例化着色器

        // 使用3个桶分别装3个LOD的实例化model矩阵
        std::unordered_map<std::string, std::vector<glm::mat4>> instancedGroups[3];
        for (auto obj : objects) {
            if (!obj->isVisible) continue;
            if (!obj->useInstancing || obj->instances.empty()) continue;

            AABB worldAABB = obj->GetWorldAABB();
            if (!frustum.isBoxVisible(worldAABB)) continue;

            for (auto& mat : obj->instances) {
                glm::vec3 pos(mat[3]);
                float distance = glm::distance(camera.Position, pos);

                int lod = 0;
                if (distance > 20.0f) lod = 2;
                else if (distance > 10.0f) lod = 1;

                instancedGroups[lod][obj->modelName].push_back(mat);
            }
        }
        // 分别渲染每个LOD的实例化
        for (int lod = 0; lod < 3; lod++) {
            for (auto& [modelName, matrices] : instancedGroups[lod]) {
                if (matrices.empty()) continue;
                Model* model = ResourceManager::GetModel(modelName);
                if (!model) continue;
                // model->DrawInstanced(*instancedShader, matrices, lod);
                if (modelName == "grass" && grassShader) {
                    model->DrawInstanced(*grassShader, matrices, lod);
                } else {
                    model->DrawInstanced(*instancedShader, matrices, lod);
                }
            }
        }

    }
}

void Renderer::RenderFloor(Camera& camera, glm::mat4 lightSpaceMatrix, glm::vec3 lightPos, glm::vec3 sunDir, bool shadowOn, float screenWidth, float screenHeight) {
    Shader* floorShader = ResourceManager::GetShader("floor");
    floorShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    floorShader->setMatrix4fv("projection", projection);
    floorShader->setMatrix4fv("view", camera.GetViewMatrix());
    floorShader->setFloat3("viewPos", camera.Position);
    floorShader->setFloat3("lightPos", lightPos);
    floorShader->setFloat3("dirLight.direction", sunDir);
    floorShader->setFloat3("dirLight.ambient", 0.005f, 0.005f, 0.005f);
    floorShader->setFloat3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    floorShader->setFloat3("dirLight.specular", 0.5f, 0.5f, 0.5f);
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

void Renderer::RenderTerrainShadow(Terrain& terrain, const glm::mat4& lightSpaceMatrix) {
    Shader* depthShader = ResourceManager::GetShader("depth");
    depthShader->use();
    depthShader->setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);
    depthShader->setMatrix4fv("model", glm::mat4(1.0f));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glCullFace(GL_FRONT);
    terrain.Draw();
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::RenderTerrain(Terrain& terrain, Camera& camera, glm::mat4 lightSpaceMatrix, glm::vec3 lightPos, glm::vec3 sunDir, bool shadowOn, float screenWidth, float screenHeight) {
    Shader* terrainShader = ResourceManager::GetShader("terrain");
    terrainShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    terrainShader->setMatrix4fv("projection", projection);
    terrainShader->setMatrix4fv("view", camera.GetViewMatrix());
    terrainShader->setFloat3("viewPos", camera.Position);
    terrainShader->setFloat3("lightPos", lightPos);
    terrainShader->setFloat3("dirLight.direction", sunDir);
    terrainShader->setFloat3("dirLight.ambient", 0.005f, 0.005f, 0.005f);
    terrainShader->setFloat3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    terrainShader->setFloat3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    terrainShader->setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);
    terrainShader->setBool("shadowOn", shadowOn);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    terrainShader->setInt("shadowMap", 10);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ResourceManager::GetTexture("ground"));
    terrainShader->setInt("terrainTexture", 0);

    terrain.Draw();
}

void Renderer::RenderWater(WaterManager& water, Camera& camera, glm::mat4 lightSpaceMatrix, glm::vec3 lightPos, glm::vec3 sunDir, bool shadowOn, float screenWidth, float screenHeight) {
    if (water.indexCount == 0) return;

    Shader* waterShader = ResourceManager::GetShader("water");
    waterShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    waterShader->setMatrix4fv("projection", projection);
    waterShader->setMatrix4fv("view", camera.GetViewMatrix());
    waterShader->setFloat3("viewPos", camera.Position);
    waterShader->setFloat3("lightPos", lightPos);
    waterShader->setFloat3("sunDir", sunDir);
    waterShader->setMatrix4fv("lightSpaceMatrix", lightSpaceMatrix);
    waterShader->setBool("shadowOn", shadowOn);

    float curTime = (float)glfwGetTime();
    waterShader->setFloat("time", curTime);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    waterShader->setInt("shadowMap", 10);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ResourceManager::GetTexture("waterCubemap"));
    waterShader->setInt("skybox", 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    water.Draw();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
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
    lightShader->setFloat3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    

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
    boxShader->setFloat3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

    // 开启线框绘制模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // 开启深度测试，避免线框交叉时显示错误
    // glEnable(GL_DEPTH_TEST); 
    glBindVertexArray(lightCubeVAO);
    for (auto obj : objects){
        if (!obj->hasCollision) continue;

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

void Renderer::RenderSpheres(const std::vector<glm::mat4>& models,
                              const glm::vec3& color,
                              Camera& camera,
                              float screenWidth,
                              float screenHeight) {
    Shader* lightShader = ResourceManager::GetShader("light");
    lightShader->use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), screenWidth / screenHeight, 0.1f, 100.0f);
    lightShader->setMatrix4fv("view", camera.GetViewMatrix());
    lightShader->setMatrix4fv("projection", projection);
    lightShader->setFloat3("lightColor", color);

    glBindVertexArray(sphereVAO);
    for (const auto& model : models) {
        lightShader->setMatrix4fv("model", model);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

