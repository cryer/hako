#pragma once
#include <vector>
#include <cstdint>

class PerlinNoise {
public:
    // 构造函数：传入种子初始化置换表
    PerlinNoise(unsigned int seed = 0);

    // 基础 3D 柏林噪声 (返回值 -1.0 ~ 1.0)
    double noise(double x, double y, double z) const;

    // 分形布朗运动 (Fractal Brownian Motion) - 用于叠加多层噪声生成地形
    // octaves: 叠加层数, persistence: 振幅衰减, lacunarity: 频率增长
    double fbm(double x, double y, double z, int octaves, double persistence, double lacunarity) const;

private:
    std::vector<int> p; // 置换表 (Permutation Table)
    
    // 数学辅助函数
    static double fade(double t);
    static double lerp(double t, double a, double b);
    static double grad(int hash, double x, double y, double z);
};