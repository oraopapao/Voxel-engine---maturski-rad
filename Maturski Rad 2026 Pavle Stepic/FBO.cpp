
#include "FBO.h"

FBO::FBO() {
	glGenFramebuffers(1, &ID);
}

void FBO::BindTexture2D(GLenum attachment, GLuint texture, GLint level) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, level);
}

void FBO::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, ID);
}
void FBO::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void FBO::Delete() {
	glDeleteFramebuffers(1, &ID);
}
