#include "Chunk.hpp"
#include <iostream>

Chunk::Chunk(int x, int z, const PerlinNoise& noiseGen) 
    : worldPos(x * CHUNK_W, 0, z * CHUNK_W) 
{
    aabb.min = glm::vec3(worldPos);
    aabb.max = glm::vec3(worldPos) + glm::vec3(CHUNK_W, CHUNK_H, CHUNK_W);

    generateTerrain(noiseGen);
    
    // 启动后台构建网格
    meshTask = std::async(std::launch::async, &Chunk::buildGreedyMesh, this);
}

Chunk::~Chunk() {
    if(meshTask.valid()) meshTask.wait();
    if(VAO) glDeleteVertexArrays(1, &VAO);
    if(VBO) glDeleteBuffers(1, &VBO);
}

void Chunk::generateTerrain(const PerlinNoise& noiseGen) {
    for(int x = 0; x < CHUNK_W; ++x) {
        for(int z = 0; z < CHUNK_W; ++z) {
            // 使用自定义的 fbm 函数
            // 坐标缩放系数 0.04 使地形起伏更平缓自然
            double n = noiseGen.fbm((worldPos.x + x) * 0.04, 0.0, (worldPos.z + z) * 0.04, 4, 0.5, 2.0);
            
            // 归一化后的 n 约在 -1 到 1 之间，变换到高度
            int h = 15 + int((n + 0.5) * 25); 
            if(h >= CHUNK_H) h = CHUNK_H - 1;
            if(h < 1) h = 1;

            for(int y = 0; y < CHUNK_H; ++y) {
                if (y > h) {
                    blocks[x][y][z] = (y <= WATER_LEVEL) ? WATER : AIR;
                } else if (y == h) {
                    blocks[x][y][z] = (y >= WATER_LEVEL) ? GRASS : DIRT;
                } else {
                    blocks[x][y][z] = DIRT;
                }
            }
        }
    }
}

void Chunk::update() {
    if (isDirty) {
        if (meshData.empty()) { indexCount = 0; return; }
        
        if (VAO == 0) glGenVertexArrays(1, &VAO);
        if (VBO == 0) glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(Vertex), meshData.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(2);

        indexCount = (GLsizei)meshData.size();
        isDirty = false;
        meshData.clear();
        meshData.shrink_to_fit();
    }
}

void Chunk::render() {
    if (indexCount > 0) {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, indexCount);
    }
}

void Chunk::rebuild() {
    meshTask = std::async(std::launch::async, &Chunk::buildGreedyMesh, this);
}



void Chunk::buildGreedyMesh() {
    std::vector<Vertex> tempMesh;
    for (int axis = 0; axis < 3; ++axis) {
        int u = (axis + 1) % 3;
        int v = (axis + 2) % 3;
        int x[3] = {0}, q[3] = {0};
        int dims[] = {CHUNK_W, CHUNK_H, CHUNK_W};
        q[axis] = 1;

        std::vector<BlockType> mask(dims[u] * dims[v]);

        for (x[axis] = -1; x[axis] < dims[axis]; ) {
            int n = 0;
            for (x[v] = 0; x[v] < dims[v]; ++x[v]) {
                for (x[u] = 0; x[u] < dims[u]; ++x[u]) {
                    BlockType b1 = (x[axis] >= 0) ? blocks[x[0]][x[1]][x[2]] : AIR;
                    BlockType b2 = (x[axis] < dims[axis]-1) ? blocks[x[0]+q[0]][x[1]+q[1]][x[2]+q[2]] : AIR;
                    
                    bool b1Solid = (b1 != AIR && b1 != WATER);
                    bool b2Solid = (b2 != AIR && b2 != WATER);
                    bool b1Water = (b1 == WATER);
                    bool b2Water = (b2 == WATER);

                    BlockType faceType = AIR;
                    if (b1Solid && b2Solid) faceType = AIR;
                    else if (b1 == AIR && b2 == AIR) faceType = AIR;
                    else if (b1Solid && b2 == AIR) faceType = b1;
                    else if (b1 == AIR && b2Solid) faceType = b2;
                    else if (b1Water && b2 == AIR) faceType = b1;
                    else if (b1 == AIR && b2Water) faceType = b2;
                    else if (b1Solid && b2Water) faceType = b1;
                    else if (b1Water && b2Solid) faceType = b2; 
                    else if (b1Water && b2Water) faceType = AIR;

                    mask[n++] = faceType;
                }
            }

            x[axis]++; 
            n = 0;
            for (int j = 0; j < dims[v]; ++j) {
                for (int i = 0; i < dims[u]; ) {
                    if (mask[n] != AIR) {
                        BlockType type = mask[n];
                        int w = 1;
                        while (i + w < dims[u] && mask[n + w] == type) w++;
                        int h = 1;
                        bool done = false;
                        while (j + h < dims[v]) {
                            for (int k = 0; k < w; ++k) 
                                if (mask[n + k + h * dims[u]] != type) { done = true; break; }
                            if (done) break;
                            h++;
                        }

                        x[u] = i; x[v] = j;
                        int dx[] = {0,0,0}; dx[axis] = -1; 
                        int rx=x[0], ry=x[1], rz=x[2];
                        BlockType blkCheck = (rx+dx[0] >= 0) ? blocks[rx+dx[0]][ry+dx[1]][rz+dx[2]] : AIR;
                        bool isBack = (blkCheck == type);

                        pushQuad(tempMesh, axis, x, w, h, u, v, type, isBack);

                        for (int l = 0; l < h; ++l)
                            for (int k = 0; k < w; ++k)
                                mask[n + k + l * dims[u]] = AIR;
                        i += w; n += w;
                    } else { i++; n++; }
                }
            }
        }
    }
    meshData = std::move(tempMesh);
    isDirty = true;
}

void Chunk::pushQuad(std::vector<Vertex>& out, int axis, int x[3], int w, int h, int u, int v, BlockType type, bool isBack) {
    glm::vec3 p(x[0]+worldPos.x, x[1]+worldPos.y, x[2]+worldPos.z);
    glm::vec3 du(0), dv(0);
    du[u] = (float)w; dv[v] = (float)h;
    glm::vec3 n(0); n[axis] = isBack ? 1.0f : -1.0f;
    glm::vec3 c = getColor(type, axis, isBack);

    glm::vec3 v0, v1, v2, v3;
    if (isBack) { v0 = p; v1 = p + du; v2 = p + du + dv; v3 = p + dv; } 
    else { v0 = p; v1 = p + dv; v2 = p + du + dv; v3 = p + du; }
    
    out.push_back({v0, n, c}); out.push_back({v1, n, c}); out.push_back({v2, n, c});
    out.push_back({v2, n, c}); out.push_back({v3, n, c}); out.push_back({v0, n, c});
}

glm::vec3 Chunk::getColor(BlockType t, int axis, bool isBack) {
    switch(t) {
        case GRASS: 
            if (axis == 1 && isBack) return {0.25f, 0.75f, 0.25f};
            return {0.45f, 0.32f, 0.20f};
        case DIRT:  return {0.45f, 0.32f, 0.20f};
        case STONE: return {0.5f, 0.5f, 0.5f};
        case WATER: return {0.2f, 0.4f, 0.85f};
        case SAND:  return {0.9f, 0.85f, 0.6f};
        default:    return {1, 0, 1};
    }
}