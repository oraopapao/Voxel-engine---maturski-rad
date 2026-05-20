#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include<glad/glad.h>

class VBO {
public:
	GLuint ID;
	VBO(khronos_uint32_t* vertices, GLsizeiptr size);
	VBO() = default;

	void Bind();
	void Unbind();
	void Delete();
};

#endif // !VBO_CLASS_H
