#ifndef GLOBAL_EBO_CLASS_H
#define GLOBAL_EBO_CLASS_H

#include<glad/glad.h>
#include<iostream>
#include<glm/glm.hpp>

class GlobalEBO {
public:
	GLuint ID;
	GlobalEBO();

	void Bind();
	void Unbind();
	void Delete();
};

#endif // !GLOBAL_EBO_CLASS_H
