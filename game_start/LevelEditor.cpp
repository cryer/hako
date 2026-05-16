#include "LevelEditor.h"
#include "Game.h" 
#include <imgui.h>
#include <algorithm>

#include <windows.h>
#include <commdlg.h>
#include <iostream>

LevelEditor& LevelEditor::Instance() {
    static LevelEditor instance;
    return instance;
}

void LevelEditor::RenderUI(Game* game) {
    if (!isVisible) return;

    ImGui::Begin("Level Editor (Map Maker)");

    // if (ImGui::Button("Save Map to JSON")) {
    //     Level::Instance().Save("assets/levels/level_03.json", game->camera.Position);
    // }
    ImGui::InputText("##savepath", savePathBuffer, sizeof(savePathBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Save Map")) {
        Level::Instance().Save(savePathBuffer, game->camera.Position);
    }

    ImGui::InputText("##modelname", modelNameBuffer, sizeof(modelNameBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Add Model")) {
        AddModelFromFile(game, modelNameBuffer);
    }

    ImGui::InputText("##trigger name", triggerNameBuffer, sizeof(triggerNameBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Add Trigger Zone")) {
        // 在编辑器里动态创建一个触发器，放在玩家摄像机前面
        // GameObject* trigger = new GameObject("New_Trigger", "", "");
        GameObject* trigger = new GameObject(triggerNameBuffer, "", "");
        trigger->isTrigger = true;
        trigger->targetLevel = "assets/levels/next_level.json";
        trigger->transform.position = game->camera.Position + game->camera.Front * 2.0f;
        trigger->localAABB.min = glm::vec3(-1.0f);
        trigger->localAABB.max = glm::vec3(1.0f);
        Level::Instance().AddObject(trigger);
    }
    
    ImGui::Separator();

    ImGui::Text("Scene Objects:");
    // 左侧：场景对象列表
    ImGui::BeginChild("ObjectList", ImVec2(150, 0), true);

    for (int i = 0; i < Level::Instance()._objects.size(); ++i) {     
        GameObject* obj = Level::Instance()._objects[i];
        bool isSelected = (selectedObject == obj);
        
        // 当点击时将其设为选中目标
        if (ImGui::Selectable(obj->name.c_str(), isSelected)) {
            selectedObject = obj;
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // 右侧：对象属性检查器 (Inspector)
    ImGui::BeginChild("Inspector", ImVec2(0, 0), true);
    if (selectedObject != nullptr) {
        ImGui::Text("Editing: %s", selectedObject->name.c_str());
        ImGui::Separator();
        
        // Transform 编辑
        ImGui::DragFloat3("Position", &selectedObject->transform.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &selectedObject->transform.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &selectedObject->transform.scale.x, 0.05f);

        ImGui::Separator();

        if (selectedObject->isTrigger) {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "[Trigger Zone]");
            
            // 编辑目标关卡路径
            char buffer[256];
            strncpy(buffer, selectedObject->targetLevel.c_str(), sizeof(buffer));
            if (ImGui::InputText("Target Level", buffer, sizeof(buffer))) {
                selectedObject->targetLevel = buffer;
            }
        } else {
            ImGui::Checkbox("Has Collision", &selectedObject->hasCollision);
        }
        

        if (ImGui::Button("Delete Object")) {
            // 从场景列表中移除并清理内存
            auto it = std::find(Level::Instance()._objects.begin(), Level::Instance()._objects.end(), selectedObject);
            if (it != Level::Instance()._objects.end()) {
                Level::Instance()._objects.erase(it);
                delete selectedObject;
                selectedObject = nullptr;
            }
        }
    } else {
        ImGui::Text("Select an object to edit its properties.");
    }
    ImGui::EndChild();

    ImGui::End();
}

void LevelEditor::AddModelFromFile(Game* game, const std::string& name) {
    // 1. 打开 Windows 文件选择对话框
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "3D Models\0*.obj;*.pmx;*.fbx;*.gltf;*.glb\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameA(&ofn)) return; // 用户取消
    std::string filepath(filename);
    // 2. 从文件路径提取模型名称（不含目录和扩展名）
    std::string nameOnly = filepath;
    size_t lastSlash = nameOnly.find_last_of("/\\");
    if (lastSlash != std::string::npos) nameOnly = nameOnly.substr(lastSlash + 1);
    size_t lastDot = nameOnly.find_last_of('.');
    std::string modelKey = (lastDot != std::string::npos) ? nameOnly.substr(0, lastDot) : nameOnly;
    std::cout<<"modelname:" << modelKey <<std::endl;
    // 3. 如果模型未加载，则加载到 ResourceManager
    if (ResourceManager::GetModel(modelKey) == nullptr) {
        ResourceManager::LoadModel(modelKey, filepath);
    }
    // 4. 创建 GameObject，放在摄像机前方 3 单位处
    GameObject* obj = new GameObject(name, modelKey, "standard");
    obj->transform.position = game->camera.Position + game->camera.Front * 3.0f;
    Model* model = ResourceManager::GetModel(modelKey);
    if (model && model->calculateAABB) {
        obj->localAABB = model->localAABB;
        obj->modelPath = filepath;
        obj->hasCollision = true;
    }
    Level::Instance().AddObject(obj);
    selectedObject = obj; // 自动选中新物体，方便立刻调整位置
}
