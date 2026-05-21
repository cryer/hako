#pragma once
#include <glm/glm.hpp>
#include "AABB.h"
#include <vector>
#include <cmath>

enum class ProjectileState {
    BOUNCING,
    ROLLING,
    STOPPED
};

struct Projectile {
    glm::vec3 position;
    glm::vec3 velocity;
    float radius = 0.25f;
    float restitution = 0.5f;
    float gravity = 20.0f;
    float airDrag = 0.3f;
    float rollFriction = 4.0f;
    float lifetime = 20.0f;
    float age = 0.0f;
    ProjectileState state = ProjectileState::BOUNCING;
    bool onGround = false;

    AABB GetAABB() const {
        return AABB::CreateFromCenterAndSize(position, glm::vec3(radius * 2.0f));
    }

    glm::mat4 GetModelMatrix() const {
        glm::mat4 mat(1.0f);
        mat = glm::translate(mat, position);
        mat = glm::scale(mat, glm::vec3(radius));
        return mat;
    }
};

struct CollisionInfo {
    bool hit = false;
    glm::vec3 normal = glm::vec3(0.0f);
};

inline CollisionInfo ResolveProjectileCollision(AABB& projectileBox, const AABB& obstacle) {
    CollisionInfo info;
    if (projectileBox.IsSeparated(obstacle)) return info;

    glm::vec3 overlap = projectileBox.GetOverlap(obstacle);
    glm::vec3 absOverlap = glm::abs(overlap);

    if (absOverlap.x < absOverlap.y && absOverlap.x < absOverlap.z) {
        float w = projectileBox.max.x - projectileBox.min.x;
        float mc = (projectileBox.min.x + projectileBox.max.x) * 0.5f;
        float oc = (obstacle.min.x + obstacle.max.x) * 0.5f;
        if (mc < oc) {
            projectileBox.max.x = obstacle.min.x;
            projectileBox.min.x = projectileBox.max.x - w;
            info.normal = glm::vec3(-1.0f, 0.0f, 0.0f);
        } else {
            projectileBox.min.x = obstacle.max.x;
            projectileBox.max.x = projectileBox.min.x + w;
            info.normal = glm::vec3(1.0f, 0.0f, 0.0f);
        }
    } else if (absOverlap.y < absOverlap.z) {
        float h = projectileBox.max.y - projectileBox.min.y;
        float mc = (projectileBox.min.y + projectileBox.max.y) * 0.5f;
        float oc = (obstacle.min.y + obstacle.max.y) * 0.5f;
        if (mc < oc) {
            projectileBox.max.y = obstacle.min.y;
            projectileBox.min.y = projectileBox.max.y - h;
            info.normal = glm::vec3(0.0f, -1.0f, 0.0f);
        } else {
            projectileBox.min.y = obstacle.max.y;
            projectileBox.max.y = projectileBox.min.y + h;
            info.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    } else {
        float d = projectileBox.max.z - projectileBox.min.z;
        float mc = (projectileBox.min.z + projectileBox.max.z) * 0.5f;
        float oc = (obstacle.min.z + obstacle.max.z) * 0.5f;
        if (mc < oc) {
            projectileBox.max.z = obstacle.min.z;
            projectileBox.min.z = projectileBox.max.z - d;
            info.normal = glm::vec3(0.0f, 0.0f, -1.0f);
        } else {
            projectileBox.min.z = obstacle.max.z;
            projectileBox.max.z = projectileBox.min.z + d;
            info.normal = glm::vec3(0.0f, 0.0f, 1.0f);
        }
    }
    info.hit = true;
    return info;
}
