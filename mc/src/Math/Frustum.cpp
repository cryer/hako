// Frustum.cpp
#include "Frustum.hpp"

void Frustum::update(const glm::mat4& viewProj) {
    const float* m = (const float*)&viewProj[0][0];
    auto set = [&](int i, float a, float b, float c, float d) {
        planes[i].normal = glm::vec3(a, b, c);
        float len = glm::length(planes[i].normal);
        planes[i].normal /= len;
        planes[i].distance = d / len;
    };
    // 提取左右下上近远平面
    set(0, m[3]+m[0], m[7]+m[4], m[11]+m[8],  m[15]+m[12]); 
    set(1, m[3]-m[0], m[7]-m[4], m[11]-m[8],  m[15]-m[12]); 
    set(2, m[3]+m[1], m[7]+m[5], m[11]+m[9],  m[15]+m[13]); 
    set(3, m[3]-m[1], m[7]-m[5], m[11]-m[9],  m[15]-m[13]); 
    set(4, m[3]+m[2], m[7]+m[6], m[11]+m[10], m[15]+m[14]); 
    set(5, m[3]-m[2], m[7]-m[6], m[11]-m[10], m[15]-m[14]); 
}

bool Frustum::isBoxVisible(const AABB& box) const {
    for (const auto& p : planes) {
        glm::vec3 positive = box.min;
        if (p.normal.x >= 0) positive.x = box.max.x;
        if (p.normal.y >= 0) positive.y = box.max.y;
        if (p.normal.z >= 0) positive.z = box.max.z;

        if (glm::dot(p.normal, positive) + p.distance < 0) return false;
    }
    return true;
}