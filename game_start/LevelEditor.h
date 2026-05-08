#pragma once
#include "Level.h"
#include "GameObject.h"


// 前向声明 Game 类
class Game;


class LevelEditor {
public:

    // 单例访问接口
    static LevelEditor& Instance();

    // 删除拷贝/移动
    LevelEditor(const LevelEditor&) = delete;
    LevelEditor& operator=(const LevelEditor&) = delete;
    LevelEditor(LevelEditor&&) = delete;
    LevelEditor& operator=(LevelEditor&&) = delete;

    bool isVisible = false;
    GameObject* selectedObject = nullptr;
    // 单例化后不再需要成员变量，直接访问 Level::Instance()
    // Level currentLevel;

    // 在你的主循环渲染 UI 阶段调用
    void RenderUI(Game* game);

private:
    // 私有构造
    LevelEditor() = default;
};