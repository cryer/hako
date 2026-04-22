#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class DebugTerminal {
public:
    DebugTerminal();
    ~DebugTerminal();

    // 核心绘制与更新函数，在主循环的 ImGui::NewFrame() 之后调用
    void Draw();

    // 添加一条日志到终端 (兼容 MSVC 的标准变参写法)
    void AddLog(const char* fmt, ...);

    bool GetVisible() const { return IsVisible; }

    // ==========================================
    // 属性绑定系统 (用于 set / get 命令)
    // 外部调用这些函数将变量的指针绑定到终端
    // ==========================================
    void BindInt(const std::string& name, int* ptr);
    void BindFloat(const std::string& name, float* ptr);
    void BindBool(const std::string& name, bool* ptr);

    // ==========================================
    // 自定义命令系统
    // 外部可以注册复杂的自定义命令行为
    // ==========================================
    using CommandCallback = std::function<void(const std::vector<std::string>& args)>;
    void RegisterCommand(const std::string& name, CommandCallback callback, const std::string& helpText = "");

private:
    void ExecuteCommand(const std::string& command_line);
    void ParseSetCommand(const std::vector<std::string>& args);
    void ParseGetCommand(const std::vector<std::string>& args);
    void PrintHelp();
    void ClearLog();

    bool IsVisible;
    char InputBuf[256];
    std::vector<std::string> Items;
    bool ScrollToBottom;

    // 内部支持的变量类型枚举
    enum class PropType { Int, Float, Bool };
    
    // 属性注册表
    struct Property {
        void* ptr;
        PropType type;
    };
    std::unordered_map<std::string, Property> Properties;

    // 命令注册表
    struct CommandDef {
        CommandCallback callback;
        std::string helpText;
    };
    std::unordered_map<std::string, CommandDef> Commands;
};