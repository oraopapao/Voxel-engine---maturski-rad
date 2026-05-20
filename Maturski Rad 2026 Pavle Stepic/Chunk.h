#ifndef CHUNK_CLASS_H
#define CHUNK_CLASS_H

#include<glad/glad.h>
#include"VAO.h"
#include"VBO.h"
#include"BlockDefs.h"
#include<bitset>
#include<glm/glm.hpp>
#include"GlobalEBO.h"

const int CHUNK_SIZE_X = 16;
const int CHUNK_SIZE_Z = 16;
const int CHUNK_SIZE_Y = 64;
const int CHUNK_VOLUME = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;

class Chunk {

public:

	Chunk(glm::vec3 position);

	std::bitset<CHUNK_VOLUME> chunkMask;
	khronos_uint8_t blocks[CHUNK_VOLUME];

	glm::vec3 Position;

	VAO VAO1;
	VBO VBO1;

	static int totalWorldQuads;
	int chunkX = 0, chunkZ = 0;
	Chunk* neighbors[4] = { nullptr, nullptr, nullptr, nullptr };

	int index(int x, int y, int z);

	void setBlock(int x, int y, int z, khronos_uint8_t type);
	bool getBlock(int x, int y, int z);

	void Generate();
	void BuildMesh(GlobalEBO& globEBO);
	void Render();
private:
	bool isTransparent(int x, int y, int z);

	enum Face : uint32_t {
		FACE_TOP = 0,
		FACE_BOTTOM = 1,
		FACE_NORTH = 2,  // +Z
		FACE_SOUTH = 3,  // -Z
		FACE_EAST = 4,  // +X
		FACE_WEST = 5   // -X
	};
	void UploadMesh(std::vector<khronos_uint32_t>& vertices, GlobalEBO& globEBO);

	uint32_t PackVertex(khronos_uint32_t x, khronos_uint32_t y, khronos_uint32_t z, Face face, khronos_uint32_t vertexId, khronos_uint32_t textureId);

	khronos_uint32_t quadCount = 0;

	inline void AddQuad(std::vector<khronos_uint32_t>& vertices, int x, int y, int z, Face face, khronos_uint8_t texLayer);

	//lookup tabela, jednom u memoriji
	struct FaceVerts {
		int8_t dx[4], dy[4], dz[4];
	};

	static constexpr FaceVerts kFaceVerts[6] = {
		// FACE_TOP
		{ {0,0,1,1}, {1,1,1,1}, {0,1,1,0} },
		// FACE_BOTTOM
		{ {0,0,1,1}, {0,0,0,0}, {1,0,0,1} },
		// FACE_NORTH (+Z)
		{ {1,1,0,0}, {0,1,1,0}, {1,1,1,1} },
		// FACE_SOUTH (-Z)
		{ {0,0,1,1}, {0,1,1,0}, {0,0,0,0} },
		// FACE_EAST (+X)
		{ {1,1,1,1}, {0,1,1,0}, {0,0,1,1} },
		// FACE_WEST (-X)
		{ {0,0,0,0}, {0,1,1,0}, {1,1,0,0} },
	};


};

#endif // !CHUNK_CLASS_H

