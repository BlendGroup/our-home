#include<lake.h>

#define tex_1k 1920, 1080

lake::lake() {
	glGenVertexArrays(1, &this->vaoEmpty);
	
	glGenTextures(1, &this->texColorReflection);
	glBindTexture(GL_TEXTURE_2D, this->texColorReflection);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, tex_1k);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenRenderbuffers(1, &this->rboDepthReflection);
	glBindRenderbuffer(GL_RENDERBUFFER, this->rboDepthReflection);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, tex_1k);
	glGenTextures(1, &this->texColorRefraction);
	glBindTexture(GL_TEXTURE_2D, this->texColorRefraction);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, tex_1k);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenTextures(1, &this->texDepthRefraction);
	glBindTexture(GL_TEXTURE_2D, this->texDepthRefraction);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, tex_1k);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glGenFramebuffers(1, &this->fboRefraction);
	glBindFramebuffer(GL_FRAMEBUFFER, this->fboRefraction);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texColorRefraction, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->texDepthRefraction, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenFramebuffers(1, &this->fboReflection);
	glBindFramebuffer(GL_FRAMEBUFFER, this->fboReflection);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texColorReflection, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->rboDepthReflection);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint lake::getReflectionTexture(void) {
	return this->texColorReflection;
}

GLuint lake::getRefractionTexture(void) {
	return this->texColorRefraction;
}

GLuint lake::getDepthTexture(void) {
	return this->texDepthRefraction;
}

void lake::setRefractionFBO(void) {
	glBindFramebuffer(GL_FRAMEBUFFER, this->fboRefraction);
	glViewport(0, 0, tex_1k);
}

void lake::setReflectionFBO(void) {
	glBindFramebuffer(GL_FRAMEBUFFER, this->fboReflection);
	glViewport(0, 0, tex_1k);
}

void lake::render() {
	glBindVertexArray(this->vaoEmpty);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}