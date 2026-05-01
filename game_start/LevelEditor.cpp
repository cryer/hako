#include "LevelEditor.h"
#include "Game.h" 
#include <imgui.h>
#include <algorithm>

void LevelEditor::RenderUI(Game* game) {
    if (!isVisible) return;

    ImGui::Begin("Level Editor (Map Maker)");

    if (ImGui::Button("Save Map to JSON")) {
        currentLevel.Save("assets/levels/level_02.json", game->sceneObjects, game->camera.Position);
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
        ImGui::Checkbox("Has Collision", &selectedObject->hasCollision);

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