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
    if (ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false)) {
        IsVisible = !IsVisible;
    }

    if (!IsVisible) return;

    ImGui::SetNextWindowSize(ImVec2(640, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Developer Terminal", &IsVisible)) {
        ImGui::End();
        return;
    }

    // 渲染日志输出区
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), true, ImGuiWindowFlags_HorizontalScrollbar)) {
        for (const auto& item : Items) {
            ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            if (item.find("[Error]") != std::string::npos) color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            else if (item.find(">") == 0) color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); // 输入的命令高亮
            
            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::TextUnformatted(item.c_str());
            ImGui::PopStyleColor();
        }

        if (ScrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ScrollToBottom = false;
    }
    ImGui::EndChild();
    ImGui::Separator();

    // 渲染输入框
    bool reclaim_focus = false;
    ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::Text("Input:"); ImGui::SameLine();
    ImGui::PushItemWidth(-1);
    if (ImGui::InputText("##terminal_input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags)) {
        std::string s = InputBuf;
        if (!s.empty()) {
            ExecuteCommand(s);
        }
        strcpy(InputBuf, "");
        reclaim_focus = true;
    }
    ImGui::PopItemWidth();

    ImGui::SetItemDefaultFocus();
    if (reclaim_focus) ImGui::SetKeyboardFocusHere(-1);

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

    // 查找并执行自定义命令
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