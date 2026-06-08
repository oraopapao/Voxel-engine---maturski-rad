#ifndef CHUNK_CLASS_H
#define CHUNK_CLASS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <bitset>
#include <vector>

#include "VAO.h"
#include "VBO.h"
#include "BlockDefs.h"
#include "GlobalEBO.h"


constexpr int CHUNK_SIZE_X = 16;
constexpr int CHUNK_SIZE_Z = 16;
constexpr int CHUNK_SIZE_Y = 256;
constexpr int CHUNK_VOLUME = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;
constexpr int WATER_LEVEL = 5;


class Chunk {
public:

    Chunk(glm::vec3 position);

    glm::vec3 Position;
    int chunkX = 0, chunkZ = 0;
    Chunk* neighbors[4] = {};

    khronos_uint8_t blocks[CHUNK_VOLUME];
    std::bitset<CHUNK_VOLUME> chunkMask;

    VAO VAO1, VAO2;
    VBO VBO1, VBO2;
    bool isInitialized = false;

    static int totalWorldQuads;

    int  index(int x, int y, int z);
    void setBlock(int x, int y, int z, khronos_uint8_t type);
    bool getBlock(int x, int y, int z);

    void Generate();
    void BuildMesh(GlobalEBO& globEBO);
    void BuildMeshCPU();
    void UploadMeshGPU(GlobalEBO& globEBO);
    void Render();
    void RenderWater();

private:

    enum Face : uint32_t {
        FACE_TOP = 0,
        FACE_BOTTOM = 1,
        FACE_NORTH = 2,  // +Z
        FACE_SOUTH = 3,  // -Z
        FACE_EAST = 4,  // +X
        FACE_WEST = 5,  // -X
    };

    struct FaceVerts { int8_t dx[4], dy[4], dz[4]; };

    static constexpr FaceVerts kFaceVerts[6] = {
        { {0,0,1,1}, {1,1,1,1}, {0,1,1,0} },  // TOP
        { {0,0,1,1}, {0,0,0,0}, {1,0,0,1} },  // BOTTOM
        { {1,1,0,0}, {0,1,1,0}, {1,1,1,1} },  // NORTH
        { {0,0,1,1}, {0,1,1,0}, {0,0,0,0} },  // SOUTH
        { {1,1,1,1}, {0,1,1,0}, {0,0,1,1} },  // EAST
        { {0,0,0,0}, {0,1,1,0}, {1,1,0,0} },  // WEST
    };

    std::vector<khronos_uint32_t> vertices;
    std::vector<khronos_uint32_t> waterVertices;
    khronos_uint32_t quadCount = 0, waterQuadCount = 0;

    bool             isTransparent(int x, int y, int z);
    uint32_t         PackVertex(khronos_uint32_t x, khronos_uint32_t y, khronos_uint32_t z, Face face, khronos_uint32_t vertexId, khronos_uint32_t textureId);
    void             UploadMesh(std::vector<khronos_uint32_t>& vertices, GlobalEBO& globEBO, VBO& vbo, VAO& vao);
    inline void      AddQuad(std::vector<khronos_uint32_t>& vertices, int x, int y, int z, Face face, khronos_uint8_t texLayer, int whichQuadCount);
    khronos_uint8_t  getHeightBlockType(int y, int height, float normalizedN, float r, int waterLevel);
};

#endif // CHUNK_CLASS_H