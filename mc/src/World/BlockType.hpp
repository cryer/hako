#pragma once
#include <cstdint>

enum BlockType : uint8_t {
    AIR = 0,
    GRASS,
    DIRT,
    STONE,
    WATER,
    SAND
};