#include <X11/keysym.h>
#include <iostream>
#include <iterator>
#include <string>

#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/vmath.h"
#include"../include/errorlog.h"
#include"../include/hdr.h"

using namespace std;

static glshaderprogram* program;
static GLuint tempVao;

HDR::HDR(GLfloat exposure, GLfloat fade, GLsizei size) {
	this->exposure = exposure;
	this->fade = fade;
	this->size = size;
}

void HDR::setupProgram(void) {
	try {
		program = new glshaderprogram({"src/shaders/hdr.vert", "src/shaders/hdr.frag"});
	} catch (string errorString) {
		throwErr(errorString);
	}
}

void HDR::init(void) {
	glGenVertexArrays(1, &tempVao);
	glBindVertexArray(tempVao);
	glGenFramebuffers(1, &this->FBO);
	glGenTextures(1, &this->Tex);
	glBindTexture(GL_TEXTURE_2D, this->Tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, this->getSize(), this->getSize(), 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenRenderbuffers(1, &this->RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->getSize(), this->getSize());

	glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->Tex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->RBO);
	const GLenum buffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throwErr("HDR Framebuffer Not Complete !!!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HDR::render(void) {
	program->use();
	// program->printUniforms();
	glUniform1i(program->getUniformLocation("hdrTex"), 0);
	glUniform1f(program->getUniformLocation("exposure"), this->exposure);
	glUniform1f(program->getUniformLocation("fade"), this->fade);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->Tex);
	glBindVertexArray(tempVao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

GLuint HDR::getFBO(void) {
	return this->FBO;
}

GLsizei HDR::getSize(void) {
	return this->size;
}

void HDR::updateExposure(GLfloat delta) {
	this->exposure += delta;
}

GLfloat HDR::getExposure() {
	return this->exposure;
}

void HDR::keyboardfunc(int key) {
	switch(key) {
	case XK_e:
		this->updateExposure(-0.1f);
		break;
	case XK_r:
		this->updateExposure(0.1f);
		break;
	}
}

void HDR::uninit(void) {
	delete program;
}