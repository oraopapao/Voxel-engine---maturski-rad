#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>
#include "Chunk.h"

inline bool isChunkVisible(const glm::mat4& projView, glm::vec3 chunkPos) {
    glm::vec3 minP = chunkPos;
    glm::vec3 maxP = chunkPos + glm::vec3(CHUNK_SIZE_X, CHUNK_SIZE_Y, CHUNK_SIZE_Z);

    glm::vec4 planes[6];
    planes[0] = glm::row(projView, 3) + glm::row(projView, 0);
    planes[1] = glm::row(projView, 3) - glm::row(projView, 0);
    planes[2] = glm::row(projView, 3) + glm::row(projView, 1);
    planes[3] = glm::row(projView, 3) - glm::row(projView, 1);
    planes[4] = glm::row(projView, 3) + glm::row(projView, 2);
    planes[5] = glm::row(projView, 3) - glm::row(projView, 2);

    for (int i = 0; i < 6; i++) {
        glm::vec3 p = minP;
        if (planes[i].x >= 0) p.x = maxP.x;
        if (planes[i].y >= 0) p.y = maxP.y;
        if (planes[i].z >= 0) p.z = maxP.z;

        if (glm::dot(glm::vec3(planes[i]), p) + planes[i].w < 0)
            return false;
    }
    return true;
}