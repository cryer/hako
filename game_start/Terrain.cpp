#include "Terrain.h"
#include <algorithm>
#include <cmath>
#include <cstring>

static inline uint32_t HashInts(int x, int z, int seed) {
    uint32_t n = (uint32_t)(x * 1619 + z * 31337 + seed * 7907);
    n = (n << 13) ^ n;
    return n * (n * n * 15731 + 789221) + 1376312589;
}

static float HashFloat(int x, int z, int seed) {
    return (HashInts(x, z, seed) & 0x7fffffff) / 2147483648.0f;
}

static float SmoothNoise(float x, float z, int seed) {
    int xi = (int)std::floor(x);
    int zi = (int)std::floor(z);
    float fx = x - (float)xi;
    float fz = z - (float)zi;
    float sx = fx * fx * (3.0f - 2.0f * fx);
    float sz = fz * fz * (3.0f - 2.0f * fz);

    float a = HashFloat(xi,     zi,     seed);
    float b = HashFloat(xi + 1, zi,     seed);
    float c = HashFloat(xi,     zi + 1, seed);
    float d = HashFloat(xi + 1, zi + 1, seed);

    float ab = a + (b - a) * sx;
    float cd = c + (d - c) * sx;
    return ab + (cd - ab) * sz;
}

float Terrain::SampleNoise(float x, float z) const {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = config.noiseScale;
    float maxValue = 0.0f;

    for (int i = 0; i < config.octaves; i++) {
        value += SmoothNoise(x * frequency, z * frequency, config.seed + i * 100) * amplitude;
        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return (value / maxValue) * config.maxHeight;
}

void Terrain::Generate(const Config& cfg) {
    config = cfg;
    BuildMesh();
}

void Terrain::Regenerate() {
    if (VAO) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        VAO = VBO = EBO = 0;
    }
    BuildMesh();
}

void Terrain::BuildMesh() {
    int res = config.resolution;
    float size = config.worldSize;
    int numVerts = res * res;
    int numQuads = (res - 1) * (res - 1);

    heightmap.resize(numVerts);
    m_normals.resize(numVerts);

    std::vector<float> verts(numVerts * 8);
    std::vector<unsigned int> indices(numQuads * 6);

    float halfSize = size * 0.5f;
    float stepWS = size / (float)(res - 1);

    for (int z = 0; z < res; z++) {
        for (int x = 0; x < res; x++) {
            int idx = z * res + x;
            float wx = -halfSize + x * stepWS;
            float wz = -halfSize + z * stepWS;
            float h = SampleNoise(wx, wz);
            heightmap[idx] = h;

            int v = idx * 8;
            verts[v + 0] = wx;
            verts[v + 1] = h;
            verts[v + 2] = wz;
            verts[v + 3] = 0.0f;
            verts[v + 4] = 1.0f;
            verts[v + 5] = 0.0f;
            verts[v + 6] = x / (float)(res - 1) * 30.0f;
            verts[v + 7] = z / (float)(res - 1) * 30.0f;
        }
    }

    // 法线
    for (int z = 0; z < res; z++) {
        for (int x = 0; x < res; x++) {
            int idx = z * res + x;

            float hL = (x > 0)       ? heightmap[idx - 1]      : heightmap[idx];
            float hR = (x < res - 1) ? heightmap[idx + 1]      : heightmap[idx];
            float hD = (z > 0)       ? heightmap[idx - res]    : heightmap[idx];
            float hU = (z < res - 1) ? heightmap[idx + res]    : heightmap[idx];

            float dx = (x > 0 && x < res - 1) ? (hR - hL) / (2.0f * stepWS) : 0.0f;
            float dz = (z > 0 && z < res - 1) ? (hU - hD) / (2.0f * stepWS) : 0.0f;

            glm::vec3 n = glm::normalize(glm::vec3(-dx, 1.0f, -dz));
            m_normals[idx] = n;

            int v = idx * 8;
            verts[v + 3] = n.x;
            verts[v + 4] = n.y;
            verts[v + 5] = n.z;
        }
    }

    // 索引
    for (int z = 0; z < res - 1; z++) {
        for (int x = 0; x < res - 1; x++) {
            int q = z * (res - 1) + x;
            int i = q * 6;
            unsigned int tl = z * res + x;
            unsigned int tr = z * res + x + 1;
            unsigned int bl = (z + 1) * res + x;
            unsigned int br = (z + 1) * res + x + 1;
            indices[i + 0] = tl; indices[i + 1] = bl; indices[i + 2] = tr;
            indices[i + 3] = tr; indices[i + 4] = bl; indices[i + 5] = br;
        }
    }

    indexCount = (int)indices.size();

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

float Terrain::GetHeight(float worldX, float worldZ) const {
    float halfSize = config.worldSize * 0.5f;
    float u = (worldX + halfSize) / config.worldSize;
    float v = (worldZ + halfSize) / config.worldSize;
    u = std::max(0.0f, std::min(1.0f, u));
    v = std::max(0.0f, std::min(1.0f, v));

    float fx = u * (float)(config.resolution - 1);
    float fz = v * (float)(config.resolution - 1);
    int x0 = (int)std::floor(fx);
    int z0 = (int)std::floor(fz);
    int x1 = std::min(x0 + 1, config.resolution - 1);
    int z1 = std::min(z0 + 1, config.resolution - 1);

    float tx = fx - (float)x0;
    float tz = fz - (float)z0;

    float h00 = heightmap[z0 * config.resolution + x0];
    float h10 = heightmap[z0 * config.resolution + x1];
    float h01 = heightmap[z1 * config.resolution + x0];
    float h11 = heightmap[z1 * config.resolution + x1];

    return h00 * (1.0f - tx) * (1.0f - tz)
         + h10 * tx * (1.0f - tz)
         + h01 * (1.0f - tx) * tz
         + h11 * tx * tz;
}

glm::vec3 Terrain::GetNormal(float worldX, float worldZ) const {
    float halfSize = config.worldSize * 0.5f;
    float u = (worldX + halfSize) / config.worldSize;
    float v = (worldZ + halfSize) / config.worldSize;
    u = std::max(0.0f, std::min(1.0f, u));
    v = std::max(0.0f, std::min(1.0f, v));

    float fx = u * (float)(config.resolution - 1);
    float fz = v * (float)(config.resolution - 1);
    int x0 = (int)std::floor(fx);
    int z0 = (int)std::floor(fz);
    int x1 = std::min(x0 + 1, config.resolution - 1);
    int z1 = std::min(z0 + 1, config.resolution - 1);

    float tx = fx - (float)x0;
    float tz = fz - (float)z0;

    glm::vec3 n00 = m_normals[z0 * config.resolution + x0];
    glm::vec3 n10 = m_normals[z0 * config.resolution + x1];
    glm::vec3 n01 = m_normals[z1 * config.resolution + x0];
    glm::vec3 n11 = m_normals[z1 * config.resolution + x1];

    glm::vec3 result = n00 * (1.0f - tx) * (1.0f - tz)
                     + n10 * tx * (1.0f - tz)
                     + n01 * (1.0f - tx) * tz
                     + n11 * tx * tz;
    return glm::normalize(result);
}

void Terrain::Draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
