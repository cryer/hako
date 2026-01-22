#pragma once
#include <glm/glm.hpp>
#include "../World/World.hpp" // 前向声明

struct RayHit {
    bool hit;
    glm::ivec3 blockPos; // 击中的方块坐标
    glm::ivec3 faceNormal; // 击中面的法线 (用于放置方块)
    float distance;
};

class Raycaster {
public:
    // DDA 算法：在体素网格中极其高效的射线步进
    static RayHit Cast(World& world, const glm::vec3& start, const glm::vec3& direction, float maxDist) {
        glm::ivec3 p = glm::floor(start);
        glm::vec3 step = glm::sign(direction);
        glm::vec3 tDelta = 1.0f / glm::abs(direction);
        glm::vec3 dist(0);

        // 初始化步进
        if (step.x > 0) dist.x = (p.x + 1 - start.x) * tDelta.x;
        else dist.x = (start.x - p.x) * tDelta.x;
        
        if (step.y > 0) dist.y = (p.y + 1 - start.y) * tDelta.y;
        else dist.y = (start.y - p.y) * tDelta.y;

        if (step.z > 0) dist.z = (p.z + 1 - start.z) * tDelta.z;
        else dist.z = (start.z - p.z) * tDelta.z;

        glm::ivec3 face(0);
        float travelled = 0;

        while (travelled < maxDist) {
            // 检查当前方块
            if (world.getBlock(p.x, p.y, p.z) != AIR && world.getBlock(p.x, p.y, p.z) != WATER) {
                return {true, p, face, travelled};
            }

            // 步进到下一个体素
            if (dist.x < dist.y) {
                if (dist.x < dist.z) {
                    p.x += (int)step.x; travelled = dist.x; dist.x += tDelta.x; face = {-step.x, 0, 0};
                } else {
                    p.z += (int)step.z; travelled = dist.z; dist.z += tDelta.z; face = {0, 0, -step.z};
                }
            } else {
                if (dist.y < dist.z) {
                    p.y += (int)step.y; travelled = dist.y; dist.y += tDelta.y; face = {0, -step.y, 0};
                } else {
                    p.z += (int)step.z; travelled = dist.z; dist.z += tDelta.z; face = {0, 0, -step.z};
                }
            }
        }
        return {false, {0,0,0}, {0,0,0}, maxDist};
    }
};