#include "WaterManager.h"
#include <algorithm>
#include <cmath>

bool WaterManager::PointInPolygon(float x, float z) const {
    const auto& poly = config.poolVertices;
    int n = (int)poly.size();
    if (n < 3) return false;

    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        float xi = poly[i].x, zi = poly[i].y;
        float xj = poly[j].x, zj = poly[j].y;
        if (((zi > z) != (zj > z)) &&
            (x < (xj - xi) * (z - zi) / (zj - zi) + xi)) {
            inside = !inside;
        }
    }
    return inside;
}

void WaterManager::Generate(const Config& cfg) {
    config = cfg;

    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        VAO = VBO = EBO = 0;
    }

    const auto& poly = config.poolVertices;
    if (poly.size() < 3) return;

    float minX = poly[0].x, maxX = poly[0].x;
    float minZ = poly[0].y, maxZ = poly[0].y;
    for (const auto& v : poly) {
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.y < minZ) minZ = v.y;
        if (v.y > maxZ) maxZ = v.y;
    }

    int res = config.gridRes;
    float stepX = (maxX - minX) / (float)(res - 1);
    float stepZ = (maxZ - minZ) / (float)(res - 1);

    int vertCount = res * res;
    std::vector<float> verts(vertCount * 8);
    for (int z = 0; z < res; z++) {
        for (int x = 0; x < res; x++) {
            int idx = z * res + x;
            float wx = minX + x * stepX;
            float wz = minZ + z * stepZ;
            int v = idx * 8;
            verts[v + 0] = wx;
            verts[v + 1] = config.waterHeight;
            verts[v + 2] = wz;
            verts[v + 3] = 0.0f;
            verts[v + 4] = 1.0f;
            verts[v + 5] = 0.0f;
            verts[v + 6] = wx * 0.1f;
            verts[v + 7] = wz * 0.1f;
        }
    }

    std::vector<unsigned int> indices;
    for (int z = 0; z < res - 1; z++) {
        for (int x = 0; x < res - 1; x++) {
            float cx = minX + (x + 0.5f) * stepX;
            float cz = minZ + (z + 0.5f) * stepZ;
            if (PointInPolygon(cx, cz)) {
                unsigned int tl = z * res + x;
                unsigned int tr = z * res + x + 1;
                unsigned int bl = (z + 1) * res + x;
                unsigned int br = (z + 1) * res + x + 1;
                indices.push_back(tl); indices.push_back(bl); indices.push_back(tr);
                indices.push_back(tr); indices.push_back(bl); indices.push_back(br);
            }
        }
    }

    indexCount = (int)indices.size();
    if (indexCount == 0) return;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void WaterManager::Draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
