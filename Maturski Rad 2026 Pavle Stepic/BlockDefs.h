#ifndef BLOCK_TYPE_H
#define BLOCK_TYPE_H

#include <glad/glad.h>

enum BlockType : khronos_uint8_t {
    BLOCK_AIR = 0,
    BLOCK_GRASS = 1,
    BLOCK_DIRT = 2,
    BLOCK_STONE = 3,
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
    //ostalo je automatski { false, 0, 0, 0 }
};

#endif // !BLOCK_TYPE_H
