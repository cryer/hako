#include "GrassManager.h"
#include "Terrain.h"
#include <cstdlib>

void GrassManager::Generate(const Terrain& terrain, const Config& cfg) {
    m_config = cfg;

    srand(cfg.seed);

    if (m_object) {
        delete m_object;
        m_object = nullptr;
    }

    m_object = new GameObject("AutoGrass", "grass", "instanced_standard");
    m_object->useInstancing = true;
    m_object->castShadow = false;
    m_object->hasCollision = false;

    float halfSize = terrain.config.worldSize * 0.5f;
    m_object->localAABB.min = glm::vec3(-halfSize, -10.0f, -halfSize);
    m_object->localAABB.max = glm::vec3(halfSize, 20.0f, halfSize);

    for (float x = -halfSize; x < halfSize; x += cfg.stepSize) {
        for (float z = -halfSize; z < halfSize; z += cfg.stepSize) {
            float jx = x + ((rand() % 1000) / 500.0f - 1.0f) * cfg.jitterRadius;
            float jz = z + ((rand() % 1000) / 500.0f - 1.0f) * cfg.jitterRadius;

            float h = terrain.GetHeight(jx, jz);

            glm::vec3 n = terrain.GetNormal(jx, jz);
            if (glm::dot(n, glm::vec3(0.0f, 1.0f, 0.0f)) < cfg.slopeThreshold) continue;

            if ((rand() % 1000) / 1000.0f > cfg.density) continue;

            glm::mat4 mat(1.0f);
            mat = glm::translate(mat, glm::vec3(jx, h, jz));
            mat = glm::rotate(mat, glm::radians((float)(rand() % 360)), glm::vec3(0.0f, 1.0f, 0.0f));
            float s = cfg.minScale + (rand() % 1000) / 1000.0f * (cfg.maxScale - cfg.minScale);
            mat = glm::scale(mat, glm::vec3(s));

            m_object->instances.push_back(mat);
        }
    }
}

int GrassManager::GetInstanceCount() const {
    return m_object ? (int)m_object->instances.size() : 0;
}
