// Player.cpp
#include "Player.hpp"
#include <iostream>

void Player::toggleMode() {
    mode = (mode == GameMode::SPECTATOR) ? GameMode::SURVIVAL : GameMode::SPECTATOR;
    velocity = glm::vec3(0.0f);
    std::cout << "Mode switched to: " << (mode == GameMode::SPECTATOR ? "Spectator" : "Survival") << std::endl;
}

AABB Player::getAABB(glm::vec3 pos) {
    return {
        pos - glm::vec3(width/2, 0, width/2), // min
        pos + glm::vec3(width/2, height, width/2) // max
    };
}

void Player::update(float dt, World& world, bool inputs[6]) {
    if (mode == GameMode::SPECTATOR) {
        // 上帝模式逻辑 (复用之前的 Camera 逻辑，稍微调整)
        float speed = 15.0f * dt;
        if(inputs[0]) camera.Pos += camera.Front * speed;
        if(inputs[1]) camera.Pos -= camera.Front * speed;
        if(inputs[2]) camera.Pos -= camera.Right * speed;
        if(inputs[3]) camera.Pos += camera.Right * speed;
        if(inputs[4]) camera.Pos += camera.WorldUp * speed;
        if(inputs[5]) camera.Pos -= camera.WorldUp * speed;
        position = camera.Pos;
    } else {
        handleSurvivalMovement(dt, world, inputs);
        camera.Pos = position + glm::vec3(0, 1.6f, 0); // 眼睛在脚上方 1.6m
    }
}

// ★★★ 核心物理与碰撞算法 ★★★
void Player::handleSurvivalMovement(float dt, World& world, bool inputs[6]) {
    // 1. 应用重力
    velocity.y -= 28.0f * dt; 

    // 2. 处理输入 (水平移动)
    float speed = 6.0f;
    glm::vec3 moveDir(0);
    // 获取水平方向的前向量
    glm::vec3 front = glm::normalize(glm::vec3(camera.Front.x, 0, camera.Front.z));
    glm::vec3 right = glm::normalize(glm::vec3(camera.Right.x, 0, camera.Right.z));

    if(inputs[0]) moveDir += front;
    if(inputs[1]) moveDir -= front;
    if(inputs[2]) moveDir -= right;
    if(inputs[3]) moveDir += right;

    if (glm::length(moveDir) > 0) moveDir = glm::normalize(moveDir);
    
    // 空气阻力/摩擦力简化
    velocity.x = moveDir.x * speed;
    velocity.z = moveDir.z * speed;

    // 跳跃
    if (inputs[4] && isGrounded) {
        velocity.y = 9.0f; 
        isGrounded = false;
    }

    // 3. 离散碰撞检测 (分别检测 X, Y, Z 轴，防止穿墙)
    glm::vec3 dMove = velocity * dt;
    
    // Y 轴碰撞
    if (checkCollision(position + glm::vec3(0, dMove.y, 0), world)) {
        if (dMove.y < 0) isGrounded = true;
        velocity.y = 0;
        dMove.y = 0;
    } else {
        isGrounded = false;
    }
    position.y += dMove.y;

    // X 轴碰撞
    if (checkCollision(position + glm::vec3(dMove.x, 0, 0), world)) {
        velocity.x = 0; dMove.x = 0;
    }
    position.x += dMove.x;

    // Z 轴碰撞
    if (checkCollision(position + glm::vec3(0, 0, dMove.z), world)) {
        velocity.z = 0; dMove.z = 0;
    }
    position.z += dMove.z;
}

bool Player::checkCollision(glm::vec3 nextPos, World& world) {
    AABB pBox = getAABB(nextPos);
    
    // 优化：只检测包围盒接触到的方块 (range min to max)
    int minX = floor(pBox.min.x); int maxX = floor(pBox.max.x);
    int minY = floor(pBox.min.y); int maxY = floor(pBox.max.y);
    int minZ = floor(pBox.min.z); int maxZ = floor(pBox.max.z);

    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            for (int z = minZ; z <= maxZ; z++) {
                BlockType b = world.getBlock(x, y, z);
                if (b != AIR && b != WATER) { // 实心方块
                    return true;
                }
            }
        }
    }
    return false;
}