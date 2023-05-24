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
static debugCamera* staticcamera;
static glshaderprogram* programRender;
static glmodel* modelTitle;

void titlescene::setupProgram() {
	programRender = new glshaderprogram({"shaders/title/render.vert", "shaders/title/render.frag"});
}

void titlescene::setupCamera() {
	staticcamera = new debugCamera(vec3(0.0f, 0.0f, 5.0f), -90.0f, 0.0f);
}

void titlescene::init() {
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
	this->t += programglobal::deltaTime;
	if(this->t >= 5.0f) {
		playNextScene();	
	}
	cout<<this->t<<endl;
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
	cout<<titlePlacer<<endl;
#endif
}

camera* titlescene::getCamera() {
	return staticcamera;
}