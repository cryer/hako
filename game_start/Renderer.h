#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include <unordered_map>
#include "camera.h"
#include "GameObject.h"
#include "AABB.h"

class Renderer {
private:
    unsigned int depthMapFBO, depthMap;
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int planeVAO, planeVBO;
    unsigned int lightCubeVAO, VBO;

    void InitPrimitives();
    void InitShadowMap();

public:
    Renderer();
    
    void RenderShadowPass(const std::vector<GameObject*>& objects, const glm::mat4& lightSpaceMatrix);
    void RenderMainPass(const std::vector<GameObject*>& objects, Camera& camera, glm::mat4 lightSpaceMatrix, glm::vec3 lightPos, glm::vec3 sunDir, bool shadowOn, float screenWidth, float screenHeight);


    void RenderFloor(Camera& camera, glm::mat4 lightSpaceMatrix, glm::vec3 lightPos, bool shadowOn, float screenWidth, float screenHeight);
    void RenderSkybox(Camera& camera, float screenWidth, float screenHeight);
    void RenderLightCube(Camera& camera, glm::vec3 lightPos,glm::vec3 sunPos, float screenWidth, float screenHeight);

    // 绘制所有AABB线框
    void RenderAABBs(const std::vector<AABB>& aabbs, Camera& camera, float screenWidth, float screenHeight);
};