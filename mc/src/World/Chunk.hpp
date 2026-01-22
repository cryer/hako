#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <future>
#include <atomic>
#include "BlockType.hpp"
#include "../Math/Frustum.hpp" // 为了 AABB
#include "../Math/PerlinNoise.hpp" // 需要噪声生成器

constexpr int CHUNK_W = 32;
constexpr int CHUNK_H = 64;
constexpr int WATER_LEVEL = 20;

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
};

class Chunk {
public:
    glm::ivec3 worldPos;
    BlockType blocks[CHUNK_W][CHUNK_H][CHUNK_W];
    
    GLuint VAO = 0, VBO = 0;
    GLsizei indexCount = 0;
    AABB aabb;
    
    // 多线程状态
    std::atomic<bool> isDirty{false};
    std::vector<Vertex> meshData;
    std::future<void> meshTask;

    // 传入全局的 PerlinNoise 引用，避免每个 Chunk 创建一个表
    Chunk(int x, int z, const PerlinNoise& noiseGen);
    ~Chunk();

    void rebuild();
    static glm::vec3 getColor(BlockType t, int axis, bool isBack);


    void update();
    void render();

private:
    void generateTerrain(const PerlinNoise& noiseGen);
    void buildGreedyMesh();
    
    void pushQuad(std::vector<Vertex>& out, int axis, int x[3], int w, int h, int u, int v, BlockType type, bool isBack);
    
};