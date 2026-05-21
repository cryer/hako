#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

struct PoolInfo {
    bool enabled = false;
    std::vector<glm::vec2> vertices;
    float floorHeight = 0.0f;
    float edgeRadius = 2.0f;
};

class Terrain {
public:
    struct Config {
        float worldSize   = 200.0f;
        int   resolution  = 256;
        float maxHeight   = 8.0f;
        float noiseScale  = 0.02f;
        int   octaves     = 4;
        int   seed        = 42;
        PoolInfo pool;
    };

    void Generate(const Config& cfg);
    void Regenerate();
    float GetHeight(float worldX, float worldZ) const;
    glm::vec3 GetNormal(float worldX, float worldZ) const;
    bool IsInsidePool(float worldX, float worldZ) const;
    void Draw();

    Config config;
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    int indexCount = 0;

private:
    std::vector<float> heightmap;
    std::vector<glm::vec3> m_normals;

    float SampleNoise(float x, float z) const;
    void BuildMesh();
};
