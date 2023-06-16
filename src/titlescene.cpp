#include "scenes/base.h"
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
#include<godrays.h>
#include<gltextureloader.h>
#include<audio.h>

using namespace std;
using namespace vmath;

#ifdef DEBUG
static modelplacer* titlePlacer;
#endif
static debugCamera* staticcamera;
static glshaderprogram* programRender;
static glmodel* modelTitle;
static godrays* godraysTitle;
static audioplayer *playerIntro;

enum tvalues {
	GODRAYS_T,
	DISPLAY_T,
	WAIT_T
};

static GLuint texGold;
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
		{GODRAYS_T, { 0.0f, 6.0f }},
		{DISPLAY_T, { 6.0f, 4.0f }},
		{WAIT_T, { 10.0f, 2.0f }},
	});

	texGold = createTexture2D("resources/textures/gold.png", GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	modelTitle = new glmodel("resources/models/blendlogo/BLEND.glb",aiProcessPreset_TargetRealtime_Quality,false);
	godraysTitle = new godrays();
	godraysTitle->setDecay(1.0f);
	godraysTitle->setDensity(0.4f);
	godraysTitle->setExposure(1.0f);
	godraysTitle->setSamples(100);
	godraysTitle->setWeight(0.02f);

	playerIntro = new audioplayer("resources/audio/Intro.wav");
#ifdef DEBUG
	titlePlacer = new modelplacer();
#endif
}

void titlescene::render() {
	programRender->use();
	glUniformMatrix4fv(programRender->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programRender->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programRender->getUniformLocation("mMat"), 1, GL_FALSE, mat4::identity());
	if((*titleevents)[DISPLAY_T] <= 0.0f) {
		glUniform3fv(programRender->getUniformLocation("L"), 1, mix(vec3(-3.0f, 0.0f, 5.0f), vec3(3.0f, 0.0f, 5.0f), (*titleevents)[GODRAYS_T]));
		glUniform2fv(programRender->getUniformLocation("lightCutOff"), 1, vec2(3.0f, 7.0f));
		glUniform1i(programRender->getUniformLocation("occlusion"), 1);
	} else {
		glUniform3fv(programRender->getUniformLocation("L"), 1, mix(vec3(-3.0f, 0.0f, 5.0f), vec3(0.0f, 0.0f, 5.0f), (*titleevents)[GODRAYS_T]));
		glUniform2fv(programRender->getUniformLocation("lightCutOff"), 1, mix(vec2(0.0f, 0.0f), vec2(40.0f, 42.0f), (*titleevents)[DISPLAY_T]));
		glUniform1i(programRender->getUniformLocation("occlusion"), 0);
	}
	glUniform1i(programRender->getUniformLocation("texDiffuse"), 0);
	glBindTextureUnit(0, texGold);
	modelTitle->draw(programRender, 1, 0, false);
	godraysTitle->setScreenSpaceCoords(programglobal::perspective * programglobal::currentCamera->matrix(), vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

void titlescene::update(void) {
	titleevents->increment();
	if((*titleevents)[WAIT_T] >= 1.0f) {
		crossfader::startSnapshot(texTitleSceneFinal);
		render();
		crossfader::endSnapshot();
		playNextScene();
	}
	if(titleevents->getT() > 0.00001f && titleevents->getT() <= 10.0f) {
		playerIntro->play();
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
	godraysTitle->renderRays();	
}