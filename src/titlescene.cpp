#include<scenes/title.h>
#include<glshaderloader.h>
#include<glmodelloader.h>
#include<global.h>
#include<assimp/postprocess.h>
#include<vmath.h>
#include<modelplacer.h>
#include<X11/keysymdef.h>
#include<iostream>
#include<debugcamera.h>

using namespace std;
using namespace vmath;

#ifdef DEBUG
static modelplacer* titlePlacer;
#endif
static GLuint fboTitleSceneFinal;
GLuint texTitleSceneFinal;
static GLuint rboTitleSceneFinal;
static debugCamera* staticcamera;
static glshaderprogram* programRender;
static glmodel* modelTitle;

#define tex_1k 1920, 1080

void titlescene::setupProgram() {
	programRender = new glshaderprogram({"shaders/title/render.vert", "shaders/title/render.frag"});
}

void titlescene::setupCamera() {
	staticcamera = new debugCamera(vec3(0.0f, 0.0f, 5.0f), -90.0f, 0.0f);
}

void titlescene::init() {
	modelTitle = new glmodel("resources/models/blendlogo/BLEND.glb",aiProcessPreset_TargetRealtime_Quality,false);
	glGenTextures(1, &texTitleSceneFinal);
	glBindTexture(GL_TEXTURE_2D, texTitleSceneFinal);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, tex_1k);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glGenRenderbuffers(1, &rboTitleSceneFinal);
	glBindRenderbuffer(GL_RENDERBUFFER, rboTitleSceneFinal);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, tex_1k);

	glGenFramebuffers(1, &fboTitleSceneFinal);
	glBindFramebuffer(GL_FRAMEBUFFER, fboTitleSceneFinal);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texTitleSceneFinal, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboTitleSceneFinal);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
#ifdef DEBUG
	titlePlacer = new modelplacer();
#endif
}

void titlescene::render() {
	programRender->use();
	glUniformMatrix4fv(programRender->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programRender->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programRender->getUniformLocation("mMat"), 1, GL_FALSE, mat4::identity());
	modelTitle->draw(programRender, 1, false);
}

void titlescene::update(void) {
	this->t += programglobal::deltaTime;
	if(this->t >= 5.0f) {
		glBindFramebuffer(GL_FRAMEBUFFER, fboTitleSceneFinal);
		glViewport(0, 0, tex_1k);
		glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
		this->render();
		playNextScene();
	}
}

void titlescene::reset(void) {
	this->t = 0.0f;
}

void titlescene::uninit() {
	delete programRender;
	delete modelTitle;
#ifdef DEBUG
	delete titlePlacer;
#endif
}

void titlescene::keyboardfunc(int key) {
#ifdef DEBUG
	titlePlacer->keyboardfunc(key);
#endif
}

camera* titlescene::getCamera() {
	return staticcamera;
}