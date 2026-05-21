#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class WaterManager {
public:
    struct Config {
        std::vector<glm::vec2> poolVertices;  // polygon outline in XZ
        float waterHeight = 0.0f;
        int gridRes = 64;
    };

    void Generate(const Config& cfg);
    void Draw();

    Config config;
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    int indexCount = 0;

private:
    bool PointInPolygon(float x, float z) const;
};
