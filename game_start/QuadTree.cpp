#include "QuadTree.h"

QuadTree::QuadTree(int pLevel, Rect2D pBounds) {
    level = pLevel;
    bounds = pBounds;
    isSplit = false;
    for (int i = 0; i < 4; i++) {
        children[i] = nullptr;
    }
}

QuadTree::~QuadTree() {
    Clear();
}

void QuadTree::Clear() {
    objects.clear();
    if (isSplit) {
        for (int i = 0; i < 4; i++) {
            delete children[i];
            children[i] = nullptr;
        }
        isSplit = false;
    }
}

// 将 3D 的 AABB 投影到 XZ 平面上
Rect2D QuadTree::Get2DBox(GameObject* obj) {
    AABB aabb = obj->GetWorldAABB();
    return {aabb.min.x, aabb.min.z, aabb.max.x, aabb.max.z};
}

void QuadTree::Split() {
    float subWidth = (bounds.maxX - bounds.minX) / 2.0f;
    float subHeight = (bounds.maxZ - bounds.minZ) / 2.0f;
    float x = bounds.minX;
    float z = bounds.minZ;

    // 四个象限划分 (NW, NE, SW, SE)
    children[0] = new QuadTree(level + 1, {x, z, x + subWidth, z + subHeight});
    children[1] = new QuadTree(level + 1, {x + subWidth, z, bounds.maxX, z + subHeight});
    children[2] = new QuadTree(level + 1, {x, z + subHeight, x + subWidth, bounds.maxZ});
    children[3] = new QuadTree(level + 1, {x + subWidth, z + subHeight, bounds.maxX, bounds.maxZ});

    isSplit = true;
}

void QuadTree::Insert(GameObject* obj) {
    Rect2D objBox = Get2DBox(obj);

    // 如果物体不在当前节点范围内，直接丢弃
    if (!bounds.Intersects(objBox)) return;

    // 如果已经分裂，尝试塞进子节点
    if (isSplit) {
        bool placedInChild = false;
        for (int i = 0; i < 4; i++) {
            // 如果物体完全被某个子区域包裹，放进去
            if (children[i]->bounds.Contains(objBox)) {
                children[i]->Insert(obj);
                return;
            }
        }
        // 如果物体正好压在线上（跨越多个子区域），就把它留在当前父节点
    }

    objects.push_back(obj);

    // 如果当前节点塞满了，且还没到最大深度，就分裂
    if (objects.size() > MAX_OBJECTS && level < MAX_LEVELS && !isSplit) {
        Split();
        
        // 尝试把当前节点的物体往下分发
        std::vector<GameObject*> temp = objects;
        objects.clear();
        for (auto* o : temp) {
            Rect2D oBox = Get2DBox(o);
            bool placed = false;
            for (int i = 0; i < 4; i++) {
                if (children[i]->bounds.Contains(oBox)) {
                    children[i]->Insert(o);
                    placed = true;
                    break;
                }
            }
            if (!placed) objects.push_back(o); // 跨线的还是留下
        }
    }
}

void QuadTree::Query(const Rect2D& range, std::vector<GameObject*>& found) {
    // 如果查询范围跟本节点都不挨着，直接返回
    if (!bounds.Intersects(range)) return;

    // 检查本节点遗留的（跨线的）物体
    for (auto* obj : objects) {
        Rect2D objBox = Get2DBox(obj);
        if (range.Intersects(objBox)) {
            found.push_back(obj);
        }
    }

    // 递归检查子节点
    if (isSplit) {
        for (int i = 0; i < 4; i++) {
            children[i]->Query(range, found);
        }
    }
}