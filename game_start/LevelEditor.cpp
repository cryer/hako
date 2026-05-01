#include "LevelEditor.h"
#include "Game.h" 
#include <imgui.h>
#include <algorithm>

void LevelEditor::RenderUI(Game* game) {
    if (!isVisible) return;

    ImGui::Begin("Level Editor (Map Maker)");

    if (ImGui::Button("Save Map to JSON")) {
        currentLevel.Save("assets/levels/level_03.json", game->sceneObjects, game->camera.Position);
    }

    ImGui::SameLine();
    if (ImGui::Button("Add Trigger Zone")) {
        // 在编辑器里动态创建一个触发器，放在玩家摄像机前面
        GameObject* trigger = new GameObject("New_Trigger", "", "");
        trigger->isTrigger = true;
        trigger->targetLevel = "assets/levels/next_level.json";
        trigger->transform.position = game->camera.Position + game->camera.Front * 2.0f;
        trigger->localAABB.min = glm::vec3(-1.0f);
        trigger->localAABB.max = glm::vec3(1.0f);
        game->AddObject(trigger);
    }
    
    ImGui::Separator();

    ImGui::Text("Scene Objects:");
    // 左侧：场景对象列表
    ImGui::BeginChild("ObjectList", ImVec2(150, 0), true);
    for (int i = 0; i < game->sceneObjects.size(); ++i) {
        GameObject* obj = game->sceneObjects[i];
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
        
        // ImGui::Separator();
        // ImGui::Checkbox("Has Collision", &selectedObject->hasCollision);

        if (ImGui::Button("Delete Object")) {
            // 从场景列表中移除并清理内存
            auto it = std::find(game->sceneObjects.begin(), game->sceneObjects.end(), selectedObject);
            if (it != game->sceneObjects.end()) {
                game->sceneObjects.erase(it);
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