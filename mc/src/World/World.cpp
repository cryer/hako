#include "World.hpp"

World::World(const PerlinNoise& noise) : noiseGen(noise) {}

void World::addChunk(int x, int z) {
    chunks[{x, z}] = std::make_unique<Chunk>(x, z, noiseGen);
}

BlockType World::getBlock(int x, int y, int z) {
    if (y < 0 || y >= CHUNK_H) return AIR;
    
    // 1.计算所属区块坐标
    int cx = (x >= 0) ? (x / CHUNK_W) : ((x + 1) / CHUNK_W - 1);
    int cz = (z >= 0) ? (z / CHUNK_W) : ((z + 1) / CHUNK_W - 1);
    
    // 2. 快速路径：检查缓存
    if (cx == lastCX && cz == lastCZ && lastAccessedChunk != nullptr) {
        int lx = x - cx * CHUNK_W;
        int lz = z - cz * CHUNK_W;
        return lastAccessedChunk->blocks[lx][y][lz];
    }

    // 3. 慢速路径：查找哈希表
    auto it = chunks.find({cx, cz});
    if (it != chunks.end()) {
        // 更新缓存
        lastCX = cx;
        lastCZ = cz;
        lastAccessedChunk = it->second.get();
        
        // 计算区块内局部坐标
        int lx = x - cx * CHUNK_W;
        int lz = z - cz * CHUNK_W;
        return it->second->blocks[lx][y][lz];
    }
    return AIR;
}

void World::setBlock(int x, int y, int z, BlockType type) {
    if (y < 0 || y >= CHUNK_H) return;

    int cx = (x >= 0) ? (x / CHUNK_W) : ((x + 1) / CHUNK_W - 1);
    int cz = (z >= 0) ? (z / CHUNK_W) : ((z + 1) / CHUNK_W - 1);

    auto it = chunks.find({cx, cz});
    if (it != chunks.end()) {
        int lx = x - cx * CHUNK_W;
        int lz = z - cz * CHUNK_W;
        
        // 修改数据
        it->second->blocks[lx][y][lz] = type;
        
        // 重建当前区块网格
        it->second->rebuild();
        
        if (lx == 0) updateChunkMesh(cx - 1, cz);
        if (lx == CHUNK_W - 1) updateChunkMesh(cx + 1, cz);
        if (lz == 0) updateChunkMesh(cx, cz - 1);
        if (lz == CHUNK_W - 1) updateChunkMesh(cx, cz + 1);
    }
}

void World::updateChunkMesh(int cx, int cz) {
    auto it = chunks.find({cx, cz});
    if (it != chunks.end()) {
        it->second->rebuild();
    }
}