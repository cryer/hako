#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "GameObject.h"
#include "Terrain.h"

class GrassManager {
public:
    // 密度、坡度阈值、步长、抖动半径、缩放范围
    struct Config {
        float density        = 0.15f;
        float slopeThreshold = 0.6f;
        float stepSize       = 1.0f;
        float jitterRadius   = 0.3f;
        float minScale       = 0.4f;
        float maxScale       = 1.0f;
        int   seed           = 42;
    };

    void Generate(const Terrain& terrain, const Config& cfg);
    int GetInstanceCount() const;
    GameObject* GetObject() const { return m_object; }

private:
    GameObject* m_object = nullptr;
    Config m_config;
};
