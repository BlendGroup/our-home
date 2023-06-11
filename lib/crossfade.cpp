#include<crossfade.h>
#include<vmath.h>
#include<global.h>

using namespace vmath;

glshaderprogram* crossfader::programCrossFade;
GLuint crossfader::emptyvao;
GLuint fboCross;
GLuint rboCross;
	
void crossfader::init() {
	crossfader::programCrossFade = new glshaderprogram({"shaders/fsquad.vert", "shaders/crossfade.frag"});
	glGenVertexArrays(1, &crossfader::emptyvao);
	glGenRenderbuffers(1, &rboCross);
	glBindRenderbuffer(GL_RENDERBUFFER, rboCross);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, tex_1k);

	glGenFramebuffers(1, &fboCross);
	glBindFramebuffer(GL_FRAMEBUFFER, fboCross);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboCross);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void crossfader::render(GLuint tex, GLfloat t) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		crossfader::programCrossFade->use();
		glUniform1f(crossfader::programCrossFade->getUniformLocation("alpha"), 1.0f - t);
		glUniform1i(crossfader::programCrossFade->getUniformLocation("texSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glBindVertexArray(crossfader::emptyvao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
}

void crossfader::startSnapshot(GLuint& tex) {
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, tex_1k);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glBindFramebuffer(GL_FRAMEBUFFER, fboCross);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	glViewport(0, 0, tex_1k);
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
}

void crossfader::endSnapshot() {
	resetFBO();
}

void crossfader::uninit() {
	glDeleteVertexArrays(1, &crossfader::emptyvao);
	delete crossfader::programCrossFade;
}
