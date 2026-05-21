#include "GrassManager.h"
#include "Terrain.h"
#include <random>

void GrassManager::Generate(const Terrain& terrain, const Config& cfg) {

    // 局部随机引擎，线程安全，无全局副作用
    std::mt19937 rng(cfg.seed);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    std::uniform_real_distribution<float> distJitter(-1.0f, 1.0f);

    if (m_object) {
        delete m_object;
        m_object = nullptr;
    }

    // m_object = new GameObject("AutoGrass", "grass", "instanced_standard");
    m_object = new GameObject("AutoGrass", "grass", "grass");
    m_object->useInstancing = true;
    m_object->castShadow = false;
    m_object->hasCollision = false;

    float halfSize = terrain.config.worldSize * 0.5f;
    m_object->localAABB.min = glm::vec3(-halfSize, -10.0f, -halfSize);
    m_object->localAABB.max = glm::vec3(halfSize, 20.0f, halfSize);

    for (float x = -halfSize; x < halfSize; x += cfg.stepSize) {
        for (float z = -halfSize; z < halfSize; z += cfg.stepSize) {
            float jx = x + distJitter(rng) * cfg.jitterRadius;
            float jz = z + distJitter(rng) * cfg.jitterRadius;

            if (terrain.IsInsidePool(jx, jz)) continue;

            float h = terrain.GetHeight(jx, jz);

            glm::vec3 n = terrain.GetNormal(jx, jz);
            if (glm::dot(n, glm::vec3(0.0f, 1.0f, 0.0f)) < cfg.slopeThreshold) continue;

            if (dist01(rng) > cfg.density) 
                continue;

            glm::mat4 mat(1.0f);
            mat = glm::translate(mat, glm::vec3(jx, h, jz));
            mat = glm::rotate(mat, glm::radians(dist01(rng) * 360.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            float s = cfg.minScale + dist01(rng) * (cfg.maxScale - cfg.minScale);
            mat = glm::scale(mat, glm::vec3(s));

            m_object->instances.push_back(mat);
        }
    }
}

int GrassManager::GetInstanceCount() const {
    return m_object ? (int)m_object->instances.size() : 0;
}
