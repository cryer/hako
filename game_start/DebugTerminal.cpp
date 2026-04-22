#include "DebugTerminal.h"
#include <imgui.h>
#include <stdio.h>
#include <stdarg.h>
#include <sstream>

DebugTerminal::DebugTerminal() {
    ClearLog();
    memset(InputBuf, 0, sizeof(InputBuf));
    IsVisible = false;

    // 注册基础命令
    RegisterCommand("clear", [this](const std::vector<std::string>&) { ClearLog(); }, "Clear terminal log");
    RegisterCommand("help", [this](const std::vector<std::string>&) { PrintHelp(); }, "List all commands and properties");
}

DebugTerminal::~DebugTerminal() {
    ClearLog();
}

void DebugTerminal::ClearLog() {
    Items.clear();
}

// MSVC 兼容的日志输出
void DebugTerminal::AddLog(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    Items.push_back(std::string(buf));
    ScrollToBottom = true;
}

void DebugTerminal::BindInt(const std::string& name, int* ptr) { Properties[name] = {ptr, PropType::Int}; }
void DebugTerminal::BindFloat(const std::string& name, float* ptr) { Properties[name] = {ptr, PropType::Float}; }
void DebugTerminal::BindBool(const std::string& name, bool* ptr) { Properties[name] = {ptr, PropType::Bool}; }

void DebugTerminal::RegisterCommand(const std::string& name, CommandCallback callback, const std::string& helpText) {
    Commands[name] = {callback, helpText};
}

void DebugTerminal::Draw() {
    // 模块内部监听 ~ 键 (ImGuiKey_GraveAccent)，完全不污染外部主程序的 GLFW 逻辑
    /*
    整体布局：
    ImGui::Begin
      ImGui::BeginChild -  ScrollingRegion控件
        ImGui::TextUnformatted函数写文本
        ImGui::TextUnformatted函数写文本
        ...
      ImGui::EndChild
      ----ImGui::Separator分割线-----
      ImGui::Text --同一行-- ImGui::InputText
    ImGui::End
    */

    // 不同于glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
    // ImGui::IsKeyPressed第二个参数false表示不重复触发（仅按键按下瞬间响应）
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false)) {
        IsVisible = !IsVisible;
    }

    if (!IsVisible) return;
    // ImGuiCond_FirstUseEver: 仅在用户首次使用该窗口时应用此尺寸，后续用户调整的大小会被记住
    ImGui::SetNextWindowSize(ImVec2(640, 400), ImGuiCond_FirstUseEver);
    // &IsVisible: 将窗口关闭按钮(×)与可见性状态绑定，用户点击关闭时自动设为false
    if (!ImGui::Begin("Developer Terminal", &IsVisible)) {
        ImGui::End();
        return;
    }

    // 渲染日志输出区
    // [布局计算] 计算底部需要预留的高度 = 元素间距 + 输入框高度(含间距)
    // 目的：确保日志滚动区域不会遮挡底部的命令输入框
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    // [日志区域] 创建可滚动的子窗口"ScrollingRegion"用于显示历史日志
    // ImVec2(0, -footer_height_to_reserve): 宽度0=填满父窗口，高度负值=从底部向上预留空间
    // true: 显示垂直滚动条, ImGuiWindowFlags_HorizontalScrollbar: 允许水平滚动
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), true, ImGuiWindowFlags_HorizontalScrollbar)) {
        // [日志渲染] 遍历所有日志条目进行逐行显示
        for (const auto& item : Items) {
            // [颜色初始化] 默认使用白色(1.0,1.0,1.0,1.0)作为文本颜色
            ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            // [错误高亮] 如果日志包含"[Error]"标记，改为淡红色(1.0,0.4,0.4)突出显示
            if (item.find("[Error]") != std::string::npos) color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            // [命令高亮] 如果日志以">"开头(表示用户输入的命令)，改为橙色(1.0,0.8,0.6)区分
            else if (item.find(">") == 0) color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
            // [样式应用] 将计算好的颜色压入ImGui样式栈，影响后续文本渲染
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            // [文本输出] 使用TextUnformatted高效渲染原始字符串(不解析格式符，性能更好)
            ImGui::TextUnformatted(item.c_str());
            // [样式恢复] 弹出颜色样式，避免影响后续其他UI元素
            ImGui::PopStyleColor();
        }

        // [自动滚动] 判断是否需要滚动到底部：
        // 条件1: ScrollToBottom标志被外部设置(如新日志添加时)
        // 条件2: 用户当前已手动滚动到最底部(避免用户查看历史时被打断)
        if (ScrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            // 设置垂直滚动位置: 1.0f表示滚动到最底部(0.0=顶部, 0.5=中间)
            ImGui::SetScrollHereY(1.0f);
        }
        // [状态重置] 无论是否执行了滚动，都重置标志位等待下次触发
        ScrollToBottom = false;
    }
    // [区域结束] 关闭日志滚动子窗口
    ImGui::EndChild();
    // [视觉分隔] 在日志区和输入区之间添加一条水平分隔线，提升界面层次感
    ImGui::Separator();

    // 渲染输入框
    // [焦点管理] 声明标志位：标记是否需要在渲染完成后重新聚焦输入框
    bool reclaim_focus = false;
    // [输入配置] 设置输入框标志：ImGuiInputTextFlags_EnterReturnsTrue
    // 效果：用户按下回车键时，InputText函数返回true，便于检测命令提交
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
    // [标签渲染] 显示"Input:"静态文本标签
    ImGui::Text("Input:"); 
    // [布局控制] SameLine()使后续输入框与标签显示在同一行
    ImGui::SameLine();
    // [宽度设置] PushItemWidth(-1): 设置后续物品宽度为-1(表示填满剩余水平空间)
    ImGui::PushItemWidth(-1);
    // [输入框创建] 创建单行文本输入框
    // "##terminal_input": 标签(##前缀隐藏标签文字，仅用于内部ID识别)
    // InputBuf: 字符数组缓冲区，存储用户输入内容
    // IM_ARRAYSIZE: 编译时计算数组大小，防止缓冲区溢出
    if (ImGui::InputText("##terminal_input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags)) {
        // [命令捕获] 当用户按下回车且输入框获得焦点时，此条件成立
        // 将C风格字符串转换为std::string便于后续处理
        std::string s = InputBuf;
        if (!s.empty()) {
            // [命令执行] 调用类成员函数解析并执行用户输入的命令(如: help, clear, log test)
            ExecuteCommand(s);
        }
        // [缓冲区清空] 使用strcpy将输入缓冲区重置为空字符串，准备接收下一条命令
        strcpy(InputBuf, "");
        // [焦点标记] 设置标志：命令执行后需要立即将键盘焦点重新给到输入框
        reclaim_focus = true;
    }
    // [宽度恢复] 弹出之前设置的物品宽度，避免影响后续其他UI控件布局
    ImGui::PopItemWidth();
    // [焦点预设] 设置默认焦点：当窗口首次激活时，自动聚焦到下一个可聚焦控件(即输入框)
    ImGui::SetItemDefaultFocus();
    // [焦点回收] 如果reclaim_focus为true(刚执行完命令)，强制将键盘焦点设回输入框
    // 参数-1: 表示聚焦到上一个物品(即刚创建的InputText控件)
    if (reclaim_focus) ImGui::SetKeyboardFocusHere(-1);
    // [窗口结束] 关闭"Developer Terminal"主窗口，完成本帧渲染
    ImGui::End();
}

void DebugTerminal::ExecuteCommand(const std::string& command_line) {
    AddLog("> %s", command_line.c_str());

    // 解析命令行（按空格分割）
    std::istringstream iss(command_line);
    std::vector<std::string> args;
    std::string token;
    while (iss >> token) args.push_back(token);

    if (args.empty()) return;

    std::string cmd = args[0];

    // 内置命令拦截
    if (cmd == "set") { ParseSetCommand(args); return; }
    if (cmd == "get") { ParseGetCommand(args); return; }

    // 查找并执行自定义命令(默认只有help，clear)
    auto it = Commands.find(cmd);
    if (it != Commands.end()) {
        it->second.callback(args);
    } else {
        AddLog("[Error] Unknown command: '%s'", cmd.c_str());
    }
}

void DebugTerminal::ParseSetCommand(const std::vector<std::string>& args) {
    if (args.size() < 3) {
        AddLog("[Error] Usage: set <property> <value>");
        return;
    }
    std::string propName = args[1];
    std::string valueStr = args[2];

    auto it = Properties.find(propName);
    if (it == Properties.end()) {
        AddLog("[Error] Property '%s' not found.", propName.c_str());
        return;
    }

    try {
        switch (it->second.type) {
            case PropType::Int:   *(int*)it->second.ptr = std::stoi(valueStr); break;
            case PropType::Float: *(float*)it->second.ptr = std::stof(valueStr); break;
            case PropType::Bool:  *(bool*)it->second.ptr = (valueStr == "1" || valueStr == "true"); break;
        }
        AddLog("Property '%s' set to %s", propName.c_str(), valueStr.c_str());
    } catch (...) {
        AddLog("[Error] Invalid value format for property '%s'", propName.c_str());
    }
}

void DebugTerminal::ParseGetCommand(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        AddLog("[Error] Usage: get <property>");
        return;
    }
    std::string propName = args[1];
    auto it = Properties.find(propName);
    if (it == Properties.end()) {
        AddLog("[Error] Property '%s' not found.", propName.c_str());
        return;
    }

    switch (it->second.type) {
        case PropType::Int:   AddLog("%s = %d", propName.c_str(), *(int*)it->second.ptr); break;
        case PropType::Float: AddLog("%s = %f", propName.c_str(), *(float*)it->second.ptr); break;
        case PropType::Bool:  AddLog("%s = %s", propName.c_str(), *(bool*)it->second.ptr ? "true" : "false"); break;
    }
}

void DebugTerminal::PrintHelp() {
    AddLog("--- Available Commands ---");
    for (const auto& cmd : Commands) {
        AddLog("  %s - %s", cmd.first.c_str(), cmd.second.helpText.c_str());
    }
    AddLog("  set <prop> <val> - Set a property value");
    AddLog("  get <prop> - Get a property value");
    
    AddLog("--- Registered Properties ---");
    for (const auto& prop : Properties) {
        const char* typeStr = (prop.second.type == PropType::Int) ? "int" : (prop.second.type == PropType::Float) ? "float" : "bool";
        AddLog("  %s (%s)", prop.first.c_str(), typeStr);
    }
}