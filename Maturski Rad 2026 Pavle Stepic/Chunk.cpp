#include "Chunk.h"
#include "FastNoiseLite.h"
#define DEBUG
#ifdef DEBUG
#include "chrono"
#endif // DEBUG


static FastNoiseLite noise = []() {
	FastNoiseLite n;
	n.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	n.SetFrequency(0.003f);
	return n;
}();

int Chunk::totalWorldQuads = 0;

Chunk::Chunk(glm::vec3 position){
	Position = position;
	chunkX = (int)(position.x / CHUNK_SIZE_X);
	chunkZ = (int)(position.z / CHUNK_SIZE_Z);
}

int Chunk::index(int x, int y, int z) {
	int i = x + y * CHUNK_SIZE_X + z * CHUNK_SIZE_X * CHUNK_SIZE_Y;
	return i;
}

void Chunk::setBlock(int x, int y, int z, khronos_uint8_t type) {
	int i = index(x, y, z);
	blocks[i] = type;
	chunkMask.set(i, type != 0);
}

bool Chunk::getBlock(int x, int y, int z) {
	return blocks[index(x,y,z)];
}

void Chunk::Generate() {
	for (int x = 0; x < CHUNK_SIZE_X; x++) {
		for (int z = 0; z < CHUNK_SIZE_Z; z++) {

			float worldX = Position.x + x;
			float worldZ = Position.z + z;
			int height = (int)(noise.GetNoise(worldX, worldZ) * 20.0f) + 35;

			for (int y = 0; y < CHUNK_SIZE_Y; y++) {
				int i = x + y * CHUNK_SIZE_X + z * CHUNK_SIZE_X * CHUNK_SIZE_Y;
				khronos_uint8_t type;
				if (y > height)          type = BLOCK_AIR;
				else if (y == height)    type = BLOCK_GRASS;
				else if (y > height - 3) type = BLOCK_DIRT;
				else                     type = BLOCK_STONE;
				blocks[i] = type;
				chunkMask.set(i, type != 0);
			}
		}
	}
}

void Chunk::BuildMesh(GlobalEBO& globEBO) {

	std::vector<khronos_uint32_t> vertices;
	vertices.reserve(CHUNK_VOLUME);
	quadCount = 0;

	for (int x = 0;x < CHUNK_SIZE_X;x++) {
		
		for (int z = 0;z < CHUNK_SIZE_Z;z++) {

			int baseIdx = x + z * CHUNK_SIZE_X * CHUNK_SIZE_Y;

			for (int y = 0;y < CHUNK_SIZE_Y;y++) {

				if (!getBlock(x, y, z)) continue;

				khronos_uint8_t b = blocks[index(x, y, z)];
				const BlockDef& def = BLOCK_DEFS[b];

				if (isTransparent(x, y + 1, z)) AddQuad(vertices, x, y, z, FACE_TOP, def.texTop);
				if (isTransparent(x, y - 1, z)) AddQuad(vertices, x, y, z, FACE_BOTTOM, def.texBottom);
				if (isTransparent(x + 1, y, z)) AddQuad(vertices, x, y, z, FACE_EAST, def.texSide);
				if (isTransparent(x - 1, y, z)) AddQuad(vertices, x, y, z, FACE_WEST, def.texSide);
				if (isTransparent(x, y, z + 1)) AddQuad(vertices, x, y, z, FACE_NORTH, def.texSide);
				if (isTransparent(x, y, z - 1)) AddQuad(vertices, x, y, z, FACE_SOUTH, def.texSide);

			}
		}
		
	}

	if (quadCount == 0) return;

	totalWorldQuads += this->quadCount;

	/*auto t1 = std::chrono::high_resolution_clock::now();*/
	UploadMesh(vertices, globEBO);
	/*auto t2 = std::chrono::high_resolution_clock::now();
	printf("Upload: %.2f ms\n", std::chrono::duration<float, std::milli>(t2 - t1).count());*/

	std::vector<khronos_uint32_t>().swap(vertices);
}

void Chunk::Render() {
	if (quadCount == 0) return;

	VAO1.Bind();
	glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, nullptr);
	VAO1.Unbind();
}

uint32_t Chunk::PackVertex(uint32_t x, uint32_t y, uint32_t z, Face face, uint32_t vertexId, uint32_t textureId) {
	return (x & 0x1F) |               // 5 bita za X
		((y & 0x1FF) << 5) |          // 9 bita za Y
		((z & 0x1F) << 14) |          // 5 bita za Z
		((face & 0x7) << 19) |        // 3 bita za stranu (lice)
		((vertexId & 0x3) << 22) |    // 2 bita za ugao (0, 1, 2, 3)
		((textureId & 0xFF) << 24);   // 8 bita za teksturu
}

void Chunk::UploadMesh(std::vector<khronos_uint32_t>& vertices, GlobalEBO& globEBO) {

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	
	VBO1 = VBO(vertices.data(), vertices.size() * sizeof(khronos_uint32_t));

	VAO1.Bind();
	VAO1.LinkVBOInt(VBO1, 0, 1, GL_UNSIGNED_INT, sizeof(khronos_uint32_t), 0);
	globEBO.Bind();

	VAO1.Unbind();
	VBO1.Unbind();
}

inline void Chunk::AddQuad(std::vector<khronos_uint32_t>& vertices, int x, int y, int z, Face face, khronos_uint8_t texLayer) {
	const auto& f = kFaceVerts[face];
	const uint32_t sharedBits = ((uint32_t)face << 19) | ((uint32_t)texLayer << 24);

	vertices.push_back(
		((uint32_t)(x + f.dx[0])) |
		((uint32_t)(y + f.dy[0]) << 5) |
		((uint32_t)(z + f.dz[0]) << 14) |
		((uint32_t)0 << 22) |
		sharedBits
	);
	vertices.push_back(
		((uint32_t)(x + f.dx[1])) |
		((uint32_t)(y + f.dy[1]) << 5) |
		((uint32_t)(z + f.dz[1]) << 14) |
		((uint32_t)1 << 22) |
		sharedBits
	);
	vertices.push_back(
		((uint32_t)(x + f.dx[2])) |
		((uint32_t)(y + f.dy[2]) << 5) |
		((uint32_t)(z + f.dz[2]) << 14) |
		((uint32_t)2 << 22) |
		sharedBits
	);
	vertices.push_back(
		((uint32_t)(x + f.dx[3])) |
		((uint32_t)(y + f.dy[3]) << 5) |
		((uint32_t)(z + f.dz[3]) << 14) |
		((uint32_t)3 << 22) |
		sharedBits
	);

	quadCount++;
}

bool Chunk::isTransparent(int x, int y, int z){
	if (y < 0 || y >= CHUNK_SIZE_Y) return true;

	if (x < 0) {
		if (neighbors[3] == nullptr) return true;
		return neighbors[3]->blocks[index(CHUNK_SIZE_X - 1, y, z)] == 0;
	}
	if (x >= CHUNK_SIZE_X) {
		if (neighbors[2] == nullptr) return true;
		return neighbors[2]->blocks[index(0, y, z)] == 0;
	}

	if (z < 0) {
		if (neighbors[1] == nullptr) return true;
		return neighbors[1]->blocks[index(x, y, CHUNK_SIZE_Z - 1)] == 0;
	}
	if (z >= CHUNK_SIZE_Z) {
		if (neighbors[0] == nullptr) return true;
		return neighbors[0]->blocks[index(x, y, 0)] == 0;
	}


	return blocks[index(x, y, z)] == 0;
}
