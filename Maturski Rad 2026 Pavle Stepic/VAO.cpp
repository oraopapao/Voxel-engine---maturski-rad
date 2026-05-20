#include "VAO.h"

VAO::VAO() {
	glGenVertexArrays(1, &ID);
}

void VAO::LinkVBO(VBO& VBO, GLuint layout, GLuint size, GLenum type, GLsizeiptr stride, GLuint offset) {
	VBO.Bind();
	glVertexAttribPointer(layout, size, type, GL_FALSE, stride, (void*)offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}

void VAO::Bind() {
	glBindVertexArray(ID);
}
void VAO::Unbind() {
	glBindVertexArray(0);
}
void VAO::Delete() {
	glDeleteVertexArrays(1, &ID);
}

void VAO::LinkVBOInt(VBO& VBO, GLuint layout, GLuint size, GLenum type, GLsizeiptr stride, GLuint offset) {
	VBO.Bind();
	glVertexAttribIPointer(layout, size, type, stride, (void*)offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}
