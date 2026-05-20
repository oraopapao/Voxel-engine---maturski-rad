#include "Chunk.h"
#include "FastNoiseLite.h"
#define DEBUG
#ifdef DEBUG
#include "chrono"
#endif // DEBUG


static FastNoiseLite noise = []() {
	FastNoiseLite n;
	n.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
	n.SetFrequency(0.001f);
	n.SetFractalType(FastNoiseLite::FractalType_FBm); // Fractal Brownian Motion
	n.SetFractalOctaves(4);        // broj slojeva
	n.SetFractalLacunarity(2.5f);  // koliko se frekvencija povećava po oktavi
	n.SetFractalGain(0.5f);        // koliko se amplituda smanjuje po oktavi
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
	chunkMask.reset();
	memset(blocks, BLOCK_AIR, CHUNK_VOLUME);

	for (int x = 0; x < CHUNK_SIZE_X; x++) {
		for (int z = 0; z < CHUNK_SIZE_Z; z++) {

			float worldX = Position.x + x;
			float worldZ = Position.z + z;

			float n = noise.GetNoise(worldX, worldZ);   // baze, niske frekvencije

			float r = (noise.GetNoise(worldX * 100, worldZ * 100) + 1.0f) * 0.5f;

			n = (n + 1.0f) / 2.0f;
			float normalizedN = n;

			n = n * n * n * n * n;
			
			n = n * 2.0f - 1.0f;

			int height = (int)(n*45)+45;



			for (int y = 0; y < CHUNK_SIZE_Y; y++) {
				int i = x + y * CHUNK_SIZE_X + z * CHUNK_SIZE_X * CHUNK_SIZE_Y;

				if (y > height) {
					if (y <= WATER_LEVEL) {
						setBlock(x, y, z, BLOCK_WATER);
					}
					continue;
				}

				// samo blokovi ispod height dolaze ovde
				khronos_uint8_t type = getHeightBlockType(y, height, normalizedN, r, WATER_LEVEL);
				blocks[i] = type;
				chunkMask.set(i, true);
			}
		}
	}
}

void Chunk::BuildMesh(GlobalEBO& globEBO) {

	std::vector<khronos_uint32_t> vertices;
	vertices.reserve(128*128);
	quadCount = 0;
	std::vector<khronos_uint32_t> waterVertices;
	waterVertices.reserve(1024);
	waterQuadCount = 0;

	for (int x = 0;x < CHUNK_SIZE_X;x++) {
		
		for (int z = 0;z < CHUNK_SIZE_Z;z++) {

			int baseIdx = x + z * CHUNK_SIZE_X * CHUNK_SIZE_Y;

			for (int y = 0;y < CHUNK_SIZE_Y;y++) {

				if (!getBlock(x, y, z)) continue;

				khronos_uint8_t b = blocks[index(x, y, z)];
				const BlockDef& def = BLOCK_DEFS[b];

				if (b == BLOCK_WATER) {
					//if (!getBlock(x, y+1, z)) AddQuad(waterVertices, x, y, z, FACE_TOP, def.texTop,waterQuadCount++);
					continue;
				}

				if (isTransparent(x, y + 1, z)) AddQuad(vertices, x, y, z, FACE_TOP, def.texTop, quadCount++);
				if (isTransparent(x, y - 1, z)) AddQuad(vertices, x, y, z, FACE_BOTTOM, def.texBottom, quadCount++);
				if (isTransparent(x + 1, y, z)) AddQuad(vertices, x, y, z, FACE_EAST, def.texSide, quadCount++);
				if (isTransparent(x - 1, y, z)) AddQuad(vertices, x, y, z, FACE_WEST, def.texSide, quadCount++);
				if (isTransparent(x, y, z + 1)) AddQuad(vertices, x, y, z, FACE_NORTH, def.texSide, quadCount++);
				if (isTransparent(x, y, z - 1)) AddQuad(vertices, x, y, z, FACE_SOUTH, def.texSide, quadCount++);
				
			}
		}
		
	}

	if (quadCount == 0) return;

	totalWorldQuads += this->quadCount;
	totalWorldQuads += this->waterQuadCount;

	/*auto t1 = std::chrono::high_resolution_clock::now();*/
	UploadMesh(vertices, globEBO, VBO1, VAO1);
	UploadMesh(waterVertices, globEBO, VBO2, VAO2);
	/*auto t2 = std::chrono::high_resolution_clock::now();
	printf("Upload: %.2f ms\n", std::chrono::duration<float, std::milli>(t2 - t1).count());*/

	std::vector<khronos_uint32_t>().swap(vertices);
	std::vector<khronos_uint32_t>().swap(waterVertices);
}

void Chunk::Render() {
	if (quadCount == 0) return;

	VAO1.Bind();
	glDrawElements(GL_TRIANGLES, quadCount * 6, GL_UNSIGNED_INT, nullptr);
	VAO1.Unbind();

}
void Chunk::RenderWater() {
	if (waterQuadCount == 0) return;

	VAO2.Bind();
	glDrawElements(GL_TRIANGLES, waterQuadCount * 6, GL_UNSIGNED_INT, nullptr);
	VAO2.Unbind();
}

uint32_t Chunk::PackVertex(uint32_t x, uint32_t y, uint32_t z, Face face, uint32_t vertexId, uint32_t textureId) {
	return (x & 0x1F) |               // 5 bita za X
		((y & 0x1FF) << 5) |          // 9 bita za Y
		((z & 0x1F) << 14) |          // 5 bita za Z
		((face & 0x7) << 19) |        // 3 bita za stranu (lice)
		((vertexId & 0x3) << 22) |    // 2 bita za ugao (0, 1, 2, 3)
		((textureId & 0xFF) << 24);   // 8 bita za teksturu
}

void Chunk::UploadMesh(std::vector<khronos_uint32_t>& vertices, GlobalEBO& globEBO, VBO& vbo, VAO& vao) {

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	
	vbo = VBO(vertices.data(), vertices.size() * sizeof(khronos_uint32_t));

	vao.Bind();
	vao.LinkVBOInt(vbo, 0, 1, GL_UNSIGNED_INT, sizeof(khronos_uint32_t), 0);
	globEBO.Bind();

	vao.Unbind();
	vbo.Unbind();
}

inline void Chunk::AddQuad(std::vector<khronos_uint32_t>& vertices, int x, int y, int z, Face face, khronos_uint8_t texLayer, int whichQuadCount) {
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

}

bool Chunk::isTransparent(int x, int y, int z){
	if (y < 0 || y >= CHUNK_SIZE_Y) return true;

	if (x < 0) {
		if (neighbors[3] == nullptr) return true;
		return !BLOCK_DEFS[neighbors[3]->blocks[index(CHUNK_SIZE_X - 1, y, z)]].opaque;
	}
	if (x >= CHUNK_SIZE_X) {
		if (neighbors[2] == nullptr) return true;
		return !BLOCK_DEFS[neighbors[2]->blocks[index(0, y, z)]].opaque;
	}

	if (z < 0) {
		if (neighbors[1] == nullptr) return true;
		return !BLOCK_DEFS[neighbors[1]->blocks[index(x, y, CHUNK_SIZE_Z - 1)]].opaque;
	}
	if (z >= CHUNK_SIZE_Z) {
		if (neighbors[0] == nullptr) return true;
		return !BLOCK_DEFS[neighbors[0]->blocks[index(x, y, 0)]].opaque;
	}


	return !BLOCK_DEFS[blocks[index(x, y, z)]].opaque;
}

inline khronos_uint8_t Chunk::getHeightBlockType(int y, int height, float normalizedN, float r, int waterLevel) {

	if (height <= waterLevel + 3) {
		if (y > height - 3) return BLOCK_SAND;
		if (y > height - 5) return BLOCK_DIRT;
		return BLOCK_STONE;
	}
	else if (normalizedN < 0.85f - r * 0.06f) {
		if (y == height)    return BLOCK_GRASS;
		if (y > height - 3) return BLOCK_DIRT;
		return BLOCK_STONE;
	}
	else if (normalizedN < 0.9f ) {
		if (y == height)    return BLOCK_GRASS_SNOW;
		if (y > height - 3) return BLOCK_DIRT;
		return BLOCK_STONE;
	}
	else {
		if (y >= height - 3) return BLOCK_SNOW;
		if (y > height - 6)  return BLOCK_DIRT;
		return BLOCK_STONE;
	}
}
