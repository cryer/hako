#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include "Chunk.hpp"
#include "../Math/PerlinNoise.hpp"

// 哈希结构体保持在头文件，因为它是模板参数
struct ChunkCoord {
    int x, z;
    bool operator==(const ChunkCoord& o) const { return x == o.x && z == o.z; }
};
struct ChunkHash {
    std::size_t operator()(const ChunkCoord& c) const { return c.x ^ (c.z << 16); }
};

class World {
public:
    std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkHash> chunks;

    World(const PerlinNoise& noise);

    void addChunk(int x, int z);
    BlockType getBlock(int x, int y, int z);
    void setBlock(int x, int y, int z, BlockType type);

private:
    const PerlinNoise& noiseGen;
    void updateChunkMesh(int cx, int cz);
    Chunk* lastAccessedChunk = nullptr;
    int lastCX = -999999;
    int lastCZ = -999999;
};