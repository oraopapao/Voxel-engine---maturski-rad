#ifndef TEXTURE_ARRAY_CLASS_H
#define TEXTURE_ARRAY_CLASS_H

#include <stb_image.h>
#include <glad/glad.h>
#include "shaderClass.h"
#include <vector>

class TextureArray {
public:
	GLuint ID;
	TextureArray(const std::vector<std::string>& imagePaths, int tileW, int tileH);

	void TexUnit(Shader shader, const char* samplerName, GLuint unit);
	void Bind();
	void Unbind();
	void Delete();
};

#endif // !TEXTURE_ARRAY_CLASS_H
