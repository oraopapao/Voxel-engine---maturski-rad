#include "GlobalEBO.h"
#include <vector>
#include "Chunk.h"

GlobalEBO::GlobalEBO() {

	// Use 16-bit indices to match GL_UNSIGNED_SHORT used in glDrawElements
	std::vector<khronos_uint32_t> indices;
	indices.reserve(CHUNK_VOLUME * 6);

	for (khronos_uint32_t i = 0; i < CHUNK_VOLUME * 6; i++) {
		khronos_uint32_t base = (i * 4);

		indices.push_back(base + 0);
		indices.push_back(base + 1);
		indices.push_back(base + 2);

		indices.push_back(base + 0);
		indices.push_back(base + 2);
		indices.push_back(base + 3);
	}

	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(khronos_uint32_t), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GlobalEBO::Bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}
void GlobalEBO::Unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void GlobalEBO::Delete() {
	glDeleteBuffers(1, &ID);
}