#ifndef BLOCK_TYPE_H
#define BLOCK_TYPE_H

#include <glad/glad.h>

enum BlockType : khronos_uint8_t {
    BLOCK_AIR = 0,
    BLOCK_GRASS = 1,
    BLOCK_DIRT = 2,
    BLOCK_STONE = 3,
    BLOCK_GRASS_SNOW = 4,
    BLOCK_SNOW = 5,
    BLOCK_WATER = 6,
    BLOCK_SAND = 7,
};

struct BlockDef {
    bool    opaque;
    khronos_uint8_t texTop;
    khronos_uint8_t texBottom;
    khronos_uint8_t texSide;
};

inline constexpr BlockDef BLOCK_DEFS[256] = {
    //opaque  top  bot  side
    { false,  0,  0,  0 },   // 0 - AIR
    { true,   0,  2,  1 },   // 1 - GRASS
    { true,   2,  2,  2 },   // 2 - DIRT
    { true,   3,  3,  3 },   // 3 - STONE
    { true,   4,  2,  5 },   // 4 - GRASS_SNOW
    { true,   4,  4,  4 },   // 5 - SNOW
    { false,  6,  6,  6 },   // 6 - WATER
    { true,   7,  7,  7 },   // 7 - SAND
    //ostalo je automatski { false, 0, 0, 0 }
};

#endif // !BLOCK_TYPE_H
