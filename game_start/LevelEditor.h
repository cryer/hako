#pragma once
#include "Level.h"
#include "GameObject.h"


// 前向声明 Game 类
class Game;


class LevelEditor {
public:
    bool isVisible = false;
    GameObject* selectedObject = nullptr;
    Level currentLevel;

    // 在你的主循环渲染 UI 阶段调用
    void RenderUI(Game* game);
};