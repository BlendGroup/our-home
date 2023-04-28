#include<scenes/lab.h>
#include<glmodelloader.h>
#include<glshaderloader.h>
#include<iostream>
#include<global.h>
#include<vmath.h>
// #include<assimp/postprocess.h>

using namespace std;
using namespace vmath;

static glmodel* labModel;
static glshaderprogram* renderModelDebug;

void labscene::setupProgram() {
	//Debug Program/////////////////
	renderModelDebug = new glshaderprogram({"shaders/debug/basictex.vert", "shaders/debug/basictex.frag"});
	////////////////////////////////
}

void labscene::setupCamera() {

}

void labscene::init() {
	labModel = new glmodel("resources/models/spaceship/SpaceLab.fbx", 0, false);
}

void labscene::render() {
	renderModelDebug->use();
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("mMat"), 1, GL_FALSE, mat4::identity());
	labModel->draw(renderModelDebug);
}

void labscene::uninit() {
	delete labModel;
}