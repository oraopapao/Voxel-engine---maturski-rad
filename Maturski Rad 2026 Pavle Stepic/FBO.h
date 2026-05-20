#ifndef FBO_CLASS_H
#define FBO_CLASS_H

#include <glad/glad.h>
#include "textureClass.h"

class FBO {
public:
	GLuint ID;
	FBO();
	
	void BindTexture2D(GLenum attachment, GLuint texture, GLint level);
	void Bind();
	void Unbind();
	void Delete();
};


#endif // !FBO_CLASS_H
