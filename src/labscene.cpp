#include<scenes/lab.h>
#include<glmodelloader.h>
#include<glshaderloader.h>
#include<iostream>
#include<global.h>
#include<vmath.h>
#include<scenecamera.h>

using namespace std;
using namespace vmath;

static glmodel* labModel;
static glshaderprogram* renderModelDebug;

void labscene::setupProgram() {
	//Debug Program/////////////////
	renderModelDebug = new glshaderprogram({"shaders/debug/basictex.vert", "shaders/debug/basictex.frag"});
	////////////////////////////////
}

sceneCamera* labscene::setupCamera() {
	PathDescriptor path;

	path.positionKeyFrames.push_back(vec3(-1.72f, 0.0f, -0.9f));
	path.positionKeyFrames.push_back(vec3(-1.72f, 0.0f, -0.7f));
	path.positionKeyFrames.push_back(vec3(-1.72f, 0.0f, -0.8f));
	path.positionKeyFrames.push_back(vec3(-1.72f, 0.0f, -0.5f));
    
	path.frontKeyFrames.push_back(vec3(-1.72f, -0.5f, -1.9f));
    path.frontKeyFrames.push_back(vec3(-1.72f, -0.5f, -1.9f));
    path.frontKeyFrames.push_back(vec3(-1.72f, -0.5f, -1.9f));
    path.frontKeyFrames.push_back(vec3(-1.72f, -0.5f, -1.9f));

	return new sceneCamera(&path);
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