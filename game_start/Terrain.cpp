#include "Terrain.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cfloat>

static bool PointInPolygon2D(float x, float z, const std::vector<glm::vec2>& poly) {
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

// 哈希函数：将整数坐标与种子映射为伪随机32位无符号整数
// 采用轻量级混合算法，保证相同输入永远输出相同结果（确定性）
static inline uint32_t HashInts(int x, int z, int seed) {
    // 第一步：线性组合，将三个输入参数初步打散
    uint32_t n = (uint32_t)(x * 1619 + z * 31337 + seed * 7907);
    // 第二步：位运算与异或混合，打乱低位相关性
    n = (n << 13) ^ n;
    // 第三步：三次多项式混合，进一步增加雪崩效应，使输出分布更均匀
    return n * (n * n * 15731 + 789221) + 1376312589;
}

// 将哈希整数转换为 [0.0, 1.0) 区间的浮点数，作为网格顶点的基础随机值
static float HashFloat(int x, int z, int seed) {
    // & 0x7fffffff 清除符号位，保留31位正整数范围
    // 除以 2^31 (2147483648.0f) 将范围映射到 [0, 1)
    return (HashInts(x, z, seed) & 0x7fffffff) / 2147483648.0f;
}

// 二维平滑值噪声核心采样函数
// 输入：浮点坐标 (x, z) 与种子
// 输出：平滑过渡的噪声值，范围约为 [0, 1]
static float SmoothNoise(float x, float z, int seed) {
    // 1. 定位当前坐标所在的整数网格单元
    int xi = (int)std::floor(x);
    int zi = (int)std::floor(z);
    // 计算坐标在该网格单元内的相对偏移量 [0.0, 1.0)
    float fx = x - (float)xi;
    float fz = z - (float)zi;

    // 2. Smoothstep 平滑插值函数：t^2 * (3 - 2t)
    // 保证在网格边界处一阶导数连续，消除生硬的折线过渡
    float sx = fx * fx * (3.0f - 2.0f * fx);
    float sz = fz * fz * (3.0f - 2.0f * fz);

    // 3. 获取当前网格单元四个顶点的伪随机值
    float a = HashFloat(xi,     zi,     seed); // 左下
    float b = HashFloat(xi + 1, zi,     seed); // 右下
    float c = HashFloat(xi,     zi + 1, seed); // 左上 
    float d = HashFloat(xi + 1, zi + 1, seed); // 右上
    // 4. 双线性插值：先沿 X 轴插值，再沿 Z 轴插值
    // 等价于 lerp(a, b, sx) 和 lerp(c, d, sx)
    float ab = a + (b - a) * sx;
    float cd = c + (d - c) * sx;
     // 最终沿 Z 轴完成插值
    return ab + (cd - ab) * sz;
}

// 二维值噪声（2D Value Noise）结合分形布朗运动（fBm）
// 属于值噪声，而柏林噪声属于梯度向量噪声，值噪声更简单，计算快内存占用小
// 地形噪声采样入口：多倍频（Octaves）分形噪声叠加
float Terrain::SampleNoise(float x, float z) const {
    float value = 0.0f;          // 累加后的噪声值
    float amplitude = 1.0f;       // 当前层振幅（对高度的影响权重）
    float frequency = config.noiseScale;  // 当前层频率（细节密度）
    float maxValue = 0.0f;              // 理论最大累加值，用于归一化
    // 循环叠加多层噪声（通常 4~8 层）
    for (int i = 0; i < config.octaves; i++) {
        // 采样当前频率下的平滑噪声，并乘以当前振幅累加
        // config.seed + i * 100 确保每层使用不同的哈希种子，避免层间相关性
        value += SmoothNoise(x * frequency, z * frequency, config.seed + i * 100) * amplitude;
        // 记录最大可能值：1 + 0.5 + 0.25 + ... = 2 - 1/2^(n-1)
        maxValue += amplitude;
        // 振幅逐层减半（高频细节权重降低）
        amplitude *= 0.5f;
        // 频率逐层翻倍（捕捉更精细的地形特征）
        frequency *= 2.0f;
    }
    // 归一化到 [0, 1] 范围，再映射到实际地形最大高度
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

    float halfSize = size * 0.5f;  // 世界坐标半宽，用于中心对称布局
    float stepWS = size / (float)(res - 1); // 世界空间步长 = 总尺寸 / (顶点数-1)

    for (int z = 0; z < res; z++) {
        for (int x = 0; x < res; x++) {
            int idx = z * res + x;
            // 将网格索引转换为世界空间坐标（原点在中心，XZ 平面）
            float wx = -halfSize + x * stepWS; // X 坐标：从 -halfSize 到 +halfSize
            float wz = -halfSize + z * stepWS; // Z 坐标：同上
            float h = SampleNoise(wx, wz); // 采样噪声函数获取该点高度
            heightmap[idx] = h; // 保存高度

            // 顶点数据布局（8 个 float / 顶点）：
            // [0] X 位置, [1] Y 高度, [2] Z 位置,
            // [3-5] 法线向量 (nx, ny, nz),
            // [6-7] UV 纹理坐标 (u, v)
            int v = idx * 8;
            verts[v + 0] = wx;
            verts[v + 1] = h;
            verts[v + 2] = wz;
            verts[v + 3] = 0.0f;
            verts[v + 4] = 1.0f;
            verts[v + 5] = 0.0f;
            // UV 坐标：将 [0, res-1] 网格索引线性映射到 [0, 30] 纹理空间
            // 30.0f 是经验值，控制纹理重复密度（避免拉伸）
            verts[v + 6] = x / (float)(res - 1) * 30.0f;
            verts[v + 7] = z / (float)(res - 1) * 30.0f;
        }
    }

    if (config.pool.enabled && config.pool.vertices.size() >= 3) {
        // ---- 高度修正：将多边形区域内的顶点高度压低为水池底 ----
        for (int z = 0; z < res; z++) {
            for (int x = 0; x < res; x++) {
                int idx = z * res + x;
                float wx = -halfSize + x * stepWS;
                float wz = -halfSize + z * stepWS;
                 // 判断当前点是否在用户定义的水池多边形内
                if (PointInPolygon2D(wx, wz, config.pool.vertices)) {
                    // 计算点到多边形每条边的最短距离（用于边缘平滑过渡）
                    float minDist = FLT_MAX;
                    int nv = (int)config.pool.vertices.size();
                    for (int i = 0; i < nv; i++) {
                        const glm::vec2& a = config.pool.vertices[i]; // 边起点
                        const glm::vec2& b = config.pool.vertices[(i + 1) % nv]; // 边终点（%nv 实现闭合）
                        glm::vec2 ab = b - a; // 边向量
                        glm::vec2 ap = glm::vec2(wx, wz) - a; // 点到起点的向量
                        // 投影参数 t：计算点 P 在直线 AB 上的垂足位置（参数化表示）
                        float t = glm::dot(ap, ab) / glm::dot(ab, ab);
                        t = glm::clamp(t, 0.0f, 1.0f); // 限制在线段范围内

                        glm::vec2 closest = a + ab * t;   // 线段上最近点
                        float dist = glm::length(ap - (ab * t)); // 实际欧氏距离
                        if (dist < minDist) minDist = dist;  // 更新最小距离
                    }

                    float targetHeight = config.pool.floorHeight;  // 水池底部目标高度
                    float originalHeight = heightmap[idx];      // 原始噪声高度
                    float radius = config.pool.edgeRadius;      // 边缘过渡区半径

                    // 根据距离应用平滑混合
                    if (minDist >= radius) {
                        // 距离足够远 → 完全使用目标高度（池底平坦）
                        heightmap[idx] = targetHeight;
                    } else {
                        // 距离 < radius → 使用 Smoothstep 在 [目标高度, 原始高度] 间插值
                        float t = minDist / radius;  // 归一化距离 [0,1]
                        float smooth = t * t * (3.0f - 2.0f * t); // Smoothstep: 3t²-2t³
                        heightmap[idx] = glm::mix(targetHeight, originalHeight, smooth);
                        // 注意：mix(a,b,t) = a*(1-t) + b*t，t=0→a, t=1→b
                        // 此处 t=0（边缘）→ 用 originalHeight；t=1（中心）→ 用 targetHeight
                    }
                }
            }
        }
        // ---- 同步更新顶点缓冲区中的 Y 坐标 ----
        for (int z = 0; z < res; z++) {
            for (int x = 0; x < res; x++) {
                int idx = z * res + x;
                int v = idx * 8;
                verts[v + 1] = heightmap[idx];
            }
        }
    }

    // 法线计算：基于高度图的有限差分
    // 使用中心差分法估算表面梯度，构造切空间法线
    for (int z = 0; z < res; z++) {
        for (int x = 0; x < res; x++) {
            int idx = z * res + x;
            // 获取四邻域高度（边界处退化为一侧差分）
            float hL = (x > 0)       ? heightmap[idx - 1]      : heightmap[idx];
            float hR = (x < res - 1) ? heightmap[idx + 1]      : heightmap[idx];
            float hD = (z > 0)       ? heightmap[idx - res]    : heightmap[idx];
            float hU = (z < res - 1) ? heightmap[idx + res]    : heightmap[idx];

            // 中心差分计算偏导数 ∂h/∂x 和 ∂h/∂z（世界空间斜率）
            // 边界处使用单侧差分避免越界，斜率设为 0（近似处理）
            float dx = (x > 0 && x < res - 1) ? (hR - hL) / (2.0f * stepWS) : 0.0f;
            float dz = (z > 0 && z < res - 1) ? (hU - hD) / (2.0f * stepWS) : 0.0f;

            // 构造表面法线：
            // 高度场表面可表示为：F(x,y,z) = y - h(x,z) = 0
            // 梯度 ∇F = (-∂h/∂x, 1, -∂h/∂z) 即为未归一化的法线方向
            glm::vec3 n = glm::normalize(glm::vec3(-dx, 1.0f, -dz));
            m_normals[idx] = n; // 保存法线供物理/后期使用
            // 写入顶点缓冲区的法线分量 [3-5]
            int v = idx * 8;
            verts[v + 3] = n.x;
            verts[v + 4] = n.y;
            verts[v + 5] = n.z;
        }
    }

    // 索引构建：四边形拆分为三角形
    // 采用 "左下→右上" 对角线分割
    for (int z = 0; z < res - 1; z++) {
        for (int x = 0; x < res - 1; x++) {
            int q = z * (res - 1) + x; // 四边形索引（行优先）
            int i = q * 6;   // 当前四边形在索引缓冲区的起始偏移（6 个 uint）

            // 四个顶点索引（行优先布局）
            unsigned int tl = z * res + x;  // Top-Left     (x, z)
            unsigned int tr = z * res + x + 1;  // Top-Right    (x+1, z)
            unsigned int bl = (z + 1) * res + x; // Bottom-Left  (x, z+1)
            unsigned int br = (z + 1) * res + x + 1; // Bottom-Right (x+1, z+1)

            // 拆分为两个三角形（逆时针顺序，保证正面朝上）：
            // 三角形 1: tl → bl → tr  （左下三角）
            // 三角形 2: tr → bl → br  （右上三角）
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

bool Terrain::IsInsidePool(float worldX, float worldZ) const {
    if (!config.pool.enabled || config.pool.vertices.size() < 3) return false;
    return PointInPolygon2D(worldX, worldZ, config.pool.vertices);
}

void Terrain::Draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
