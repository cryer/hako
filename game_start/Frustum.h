#pragma once
#include <glm/glm.hpp>
#include <array>
#include "AABB.h"


class Frustum {
public:
    struct Plane { glm::vec3 normal; float distance; };
    std::array<Plane, 6> planes;

    void update(const glm::mat4& viewProj);
    bool isBoxVisible(const AABB& box) const;
};