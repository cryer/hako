#include "AABB.h"
#include <iostream>

AABB AABB::GetTransformed(const glm::mat4& transform) const {
    // 1. 获取本地AABB的8个角顶点
    glm::vec3 corners[8] = {
        glm::vec3(min.x, min.y, min.z),
        glm::vec3(max.x, min.y, min.z),
        glm::vec3(min.x, max.y, min.z),
        glm::vec3(max.x, max.y, min.z),
        glm::vec3(min.x, min.y, max.z),
        glm::vec3(max.x, min.y, max.z),
        glm::vec3(min.x, max.y, max.z),
        glm::vec3(max.x, max.y, max.z)
    };

    AABB transformedAABB;
    // 2. 将8个点进行Model矩阵变换到世界空间，并动态计算新的包围盒
    for (int i = 0; i < 8; ++i) {
        glm::vec4 worldPos = transform * glm::vec4(corners[i], 1.0f);
        transformedAABB.Expand(glm::vec3(worldPos));
    }
    
    return transformedAABB;
}

// ===== 碰撞响应：将 player 推出碰撞体，返回是否发生碰撞 =====
// 同时需要保证包围盒尺寸不变
bool ResolveCollision(AABB& moving, const AABB& staticObs) {
    if (moving.IsSeparated(staticObs)) return false;

    // std::cout << "collison happend \n";
    
    glm::vec3 overlap = moving.GetOverlap(staticObs);
    glm::vec3 absOverlap = glm::abs(overlap);
    
    // 找出最小重叠轴
    if (absOverlap.x < absOverlap.y && absOverlap.x < absOverlap.z) {
        // X 轴解决
        if (overlap.x > 0) {
            float movingCenterX = (moving.min.x + moving.max.x) * 0.5f;
            float obsCenterX = (staticObs.min.x + staticObs.max.x) * 0.5f;
            if (movingCenterX < obsCenterX) {
                // 从左侧撞：移动右边界贴障碍物左边界
                float width = moving.max.x - moving.min.x;  // 保存原始宽度
                moving.max.x = staticObs.min.x;
                moving.min.x = moving.max.x - width;         // 保持尺寸
            } else {
                float width = moving.max.x - moving.min.x;
                moving.min.x = staticObs.max.x;
                moving.max.x = moving.min.x + width;
            }
        }
    } 
    else if (absOverlap.y < absOverlap.z) {
        // Y 轴解决（地面/天花板）
        if (overlap.y > 0) {
            float movingCenterY = (moving.min.y + moving.max.y) * 0.5f;
            float obsCenterY = (staticObs.min.y + staticObs.max.y) * 0.5f;
            if (movingCenterY < obsCenterY) {
                float height = moving.max.y - moving.min.y;
                moving.max.y = staticObs.min.y;  // 站在地面上
                moving.min.y = moving.max.y - height;
            } else {
                float height = moving.max.y - moving.min.y;
                moving.min.y = staticObs.max.y;  // 头顶撞到
                moving.max.y = moving.min.y + height;
            }
        }
    } 
    else {
        // Z 轴解决（前后）
        if (overlap.z > 0) {
            float movingCenterZ = (moving.min.z + moving.max.z) * 0.5f;
            float obsCenterZ = (staticObs.min.z + staticObs.max.z) * 0.5f;
            if (movingCenterZ < obsCenterZ) {
                float depth = moving.max.z - moving.min.z;
                moving.max.z = staticObs.min.z;
                moving.min.z = moving.max.z - depth;
            } else {
                float depth = moving.max.z - moving.min.z;
                moving.min.z = staticObs.max.z;
                moving.max.z = moving.min.z + depth;
            }
        }
    }
    return true;
}