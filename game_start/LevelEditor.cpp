#include "LevelEditor.h"
#include "Game.h" 
#include <imgui.h>
#include <algorithm>
#include <filesystem> 

#include "ImGuiFileDialog.h"


LevelEditor& LevelEditor::Instance() {
    static LevelEditor instance;
    return instance;
}

void LevelEditor::RenderUI(Game* game) {
    if (!isVisible) return;

    ImGui::Begin("Level Editor (Map Maker)");

    ImGui::InputText("##savepath", savePathBuffer, sizeof(savePathBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Save Map")) {
        Level::Instance().Save(savePathBuffer, game->camera.Position);
    }

    ImGui::InputText("##modelname", modelNameBuffer, sizeof(modelNameBuffer));
    ImGui::SameLine();

    if (ImGui::Button("Add Model")) {
        IGFD::FileDialogConfig config;
        config.path = ".";
        config.flags = ImGuiFileDialogFlags_Modal;
        ImGuiFileDialog::Instance()->OpenDialog("ChooseModelDlg", "Choose Model File",
            ".obj,.pmx,.fbx,.gltf,.glb", config);
    }

    ImGui::InputText("##trigger name", triggerNameBuffer, sizeof(triggerNameBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Add Trigger Zone")) {
        // 在编辑器里动态创建一个触发器，放在玩家摄像机前面
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

    // --- ImGuiFileDialog 文件选择 ---
    if (ImGuiFileDialog::Instance()->Display("ChooseModelDlg")) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            // 绝对地址
            std::string filepath = ImGuiFileDialog::Instance()->GetFilePathName();
            // 需要转成相对地址，否则换机器就出错
            try {
                std::filesystem::path absPath(filepath);
                std::filesystem::path relPath = std::filesystem::relative(absPath);
                filepath = relPath.generic_string();  // 使用斜杠而不是windows的反斜杠
            } catch (const std::exception&) {
                // 
            }
            std::string nameOnly = filepath;
            size_t lastSlash = nameOnly.find_last_of("/\\");
            if (lastSlash != std::string::npos) nameOnly = nameOnly.substr(lastSlash + 1);
            size_t lastDot = nameOnly.find_last_of('.');
            std::string modelKey = (lastDot != std::string::npos) ? nameOnly.substr(0, lastDot) : nameOnly;
            if (ResourceManager::GetModel(modelKey) == nullptr) {
                ResourceManager::LoadModel(modelKey, filepath);
            }
            GameObject* obj = new GameObject(modelNameBuffer, modelKey, "standard");
            obj->modelPath = filepath;
            obj->transform.position = game->camera.Position + game->camera.Front * 3.0f;
            Model* model = ResourceManager::GetModel(modelKey);
            if (model && model->calculateAABB) {
                obj->localAABB = model->localAABB;
                obj->hasCollision = true;
            }
            Level::Instance().AddObject(obj);
            selectedObject = obj;
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

