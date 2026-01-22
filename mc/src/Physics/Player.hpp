// Player.hpp
#pragma once
#include "../Graphics/Camera.hpp"
#include "../World/World.hpp"

enum class GameMode { SPECTATOR, SURVIVAL };

class Player {
public:
    Camera camera;
    GameMode mode = GameMode::SPECTATOR;
    
    // 物理属性
    glm::vec3 velocity{0.0f};
    glm::vec3 position;
    bool isGrounded = false;

    // 碰撞箱尺寸
    const float width = 0.6f;
    const float height = 1.8f;

    // 物品栏
    BlockType selectedBlock = STONE;

    Player(glm::vec3 pos) : camera(pos), position(pos) {}

    void update(float dt, World& world, bool inputs[6]); // Inputs: W,S,A,D,Space,Ctrl
    void toggleMode();

private:
    void handleSurvivalMovement(float dt, World& world, bool inputs[6]);
    bool checkCollision(glm::vec3 nextPos, World& world);
    AABB getAABB(glm::vec3 pos);
};