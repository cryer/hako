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


    // 保存路径输入框缓冲区
    char savePathBuffer[256] = "assets/levels/level_N.json";
    // 保存触发器名缓冲区
    char triggerNameBuffer[256] = "New_Trigger_N";
    // 模型名缓冲区
    char modelNameBuffer[256] = "name before add!";
    // 打开文件对话框并添加模型
    void AddModelFromFile(Game* game, const std::string& name);

    // 在你的主循环渲染 UI 阶段调用
    void RenderUI(Game* game);

private:
    // 私有构造
    LevelEditor() = default;
};