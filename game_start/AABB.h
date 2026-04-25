#pragma once
#include <glm/glm.hpp>
#include <limits>
#include <vector>


// 最简单的AABB碰撞检测
class AABB {
public:
    glm::vec3 min;
    glm::vec3 max;

    // 默认构造：初始化为"反向无限大"，方便后续向外扩展
    AABB() {
        min = glm::vec3(std::numeric_limits<float>::max());
        max = glm::vec3(-std::numeric_limits<float>::max());
    }

    // 已知最大最小值的构造
    AABB(const glm::vec3& minPoint, const glm::vec3& maxPoint) : min(minPoint), max(maxPoint) {}

    // 扩充包围盒（包含新加入的点） - 用于在加载顶点时动态计算
    void Expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    // 将另一个包围盒合并进当前包围盒
    void Merge(const AABB& other) {
        min = glm::min(min, other.min);
        max = glm::max(max, other.max);
    }

    // 碰撞检测函数：检测两个AABB是否相交
    bool Intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }

    // （性能优化）：当模型发生平移、旋转、缩放时，不需要重新遍历成千上万个顶点！
    // 只需要将局部的8个AABB顶点进行矩阵变换，再找出新的Min和Max即可。极大地优化了性能。
    AABB GetTransformed(const glm::mat4& transform) const;

    // 供外部直接创建一个具有碰撞体积的立方体（空气墙 / 摄像机实体等）
    static AABB CreateFromCenterAndSize(const glm::vec3& center, const glm::vec3& size) {
        glm::vec3 halfSize = size * 0.5f;
        return AABB(center - halfSize, center + halfSize);
    }

    // 计算两个 AABB 在各轴上的重叠量（正数=重叠，负数=分离）
    glm::vec3 GetOverlap(const AABB& other) const {
        glm::vec3 overlap;
        overlap.x = std::min(max.x, other.max.x) - std::max(min.x, other.min.x);
        overlap.y = std::min(max.y, other.max.y) - std::max(min.y, other.min.y);
        overlap.z = std::min(max.z, other.max.z) - std::max(min.z, other.min.z);
        return overlap;
    }

    // 判断是否完全分离（优化：提前退出）
    bool IsSeparated(const AABB& other) const {
        return (max.x < other.min.x || min.x > other.max.x) ||
               (max.y < other.min.y || min.y > other.max.y) ||
               (max.z < other.min.z || min.z > other.max.z);
    }

};


bool ResolveCollision(AABB& player, const AABB& obstacle);