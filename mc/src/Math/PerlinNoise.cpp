#include "PerlinNoise.hpp"
#include <numeric>
#include <random>
#include <cmath>
#include <algorithm>

PerlinNoise::PerlinNoise(unsigned int seed) {
    p.resize(512);
    // 初始化 0-255
    std::iota(p.begin(), p.begin() + 256, 0);
    // 根据种子打乱
    std::default_random_engine engine(seed);
    std::shuffle(p.begin(), p.begin() + 256, engine);
    // 复制一份以避免取模运算时的溢出处理
    for (int i = 0; i < 256; ++i) p[256 + i] = p[i];
}

double PerlinNoise::fade(double t) { 
    // Smootherstep: 6t^5 - 15t^4 + 10t^3 
    return t * t * t * (t * (t * 6 - 15) + 10); 
}

double PerlinNoise::lerp(double t, double a, double b) { 
    return a + t * (b - a); 
}

double PerlinNoise::grad(int hash, double x, double y, double z) {
    // 将哈希值转换为 12 个梯度向量之一
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::noise(double x, double y, double z) const {
    // 找到所在的单位立方体
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;

    // 找到相对坐标
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    // 计算平滑曲线值
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    // 哈希查找梯度
    int A = p[X] + Y, AA = p[A] + Z, AB = p[A + 1] + Z;
    int B = p[X + 1] + Y, BA = p[B] + Z, BB = p[B + 1] + Z;

    // 三线性插值 (Trilinear Interpolation)
    return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),  grad(p[BA], x - 1, y, z)),
                           lerp(u, grad(p[AB], x, y - 1, z),  grad(p[BB], x - 1, y - 1, z))),
                   lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),  grad(p[BA + 1], x - 1, y, z - 1)),
                           lerp(u, grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1))));
}

double PerlinNoise::fbm(double x, double y, double z, int octaves, double persistence, double lacunarity) const {
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;  // 用于归一化

    for (int i = 0; i < octaves; i++) {
        total += noise(x * frequency, y * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxValue; // 归一化到 -1 ~ 1
}