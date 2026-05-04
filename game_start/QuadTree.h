#pragma once
#include <vector>
#include "GameObject.h"
#include "AABB.h"

// 2D 边界框（用于 XZ 平面）
struct Rect2D {
    float minX, minZ;
    float maxX, maxZ;

    // 检测两个 2D 矩形是否相交
    bool Intersects(const Rect2D& other) const {
        return !(other.minX > maxX || other.maxX < minX || 
                 other.minZ > maxZ || other.maxZ < minZ);
    }
    
    // 检测是否完全包含另一个矩形
    bool Contains(const Rect2D& other) const {
        return (other.minX >= minX && other.maxX <= maxX && 
                other.minZ >= minZ && other.maxZ <= maxZ);
    }
};

class QuadTree {
private:
    static const int MAX_OBJECTS = 10; // 每个节点最多容纳的物体数
    static const int MAX_LEVELS = 5;   // 树的最大深度

    int level;
    Rect2D bounds;
    std::vector<GameObject*> objects;
    QuadTree* children[4]; // NW, NE, SW, SE
    bool isSplit;

    void Split();
    Rect2D Get2DBox(GameObject* obj);

public:
    QuadTree(int pLevel, Rect2D pBounds);
    ~QuadTree();

    void Clear();
    void Insert(GameObject* obj);
    
    // 查询与目标区域相交的所有物体
    void Query(const Rect2D& range, std::vector<GameObject*>& found);
};