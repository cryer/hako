// Frustum.hpp
#pragma once
#include <glm/glm.hpp>
#include <array>

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

class Frustum {
public:
    struct Plane { glm::vec3 normal; float distance; };
    std::array<Plane, 6> planes;

    void update(const glm::mat4& viewProj);
    bool isBoxVisible(const AABB& box) const;
};