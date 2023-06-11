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
#include<eventmanager.h>
#include<crossfade.h>

using namespace std;
using namespace vmath;

#ifdef DEBUG
static modelplacer* titlePlacer;
#endif
static debugCamera* staticcamera;
static glshaderprogram* programRender;
static glmodel* modelTitle;

enum tvalues {
	RENDERTITLE_T
};

GLuint texTitleSceneFinal;
static eventmanager* titleevents;

void titlescene::setupProgram() {
	programRender = new glshaderprogram({"shaders/title/render.vert", "shaders/title/render.frag"});
}

void titlescene::setupCamera() {
	staticcamera = new debugCamera(vec3(0.0f, 0.0f, 5.0f), -90.0f, 0.0f);
}

void titlescene::init() {
	titleevents = new eventmanager({
		{RENDERTITLE_T, { 0.0f, 2.0f }}
	});

	modelTitle = new glmodel("resources/models/blendlogo/BLEND.glb",aiProcessPreset_TargetRealtime_Quality,false);
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
	titleevents->increment();
	if((*titleevents)[RENDERTITLE_T] >= 1.0f) {
		crossfader::startSnapshot(texTitleSceneFinal);
		render();
		crossfader::endSnapshot();
		playNextScene();
	}
}

void titlescene::reset(void) {
	titleevents->resetT();
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

void titlescene::crossfade() {
	
}