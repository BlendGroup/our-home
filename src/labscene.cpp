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

	path.positionKeyFrames.push_back(vec3(0.0f, 20.0f, 35.0f));
    path.positionKeyFrames.push_back(vec3(5.0f, 15.0f, 15.0f));
    path.positionKeyFrames.push_back(vec3(-5.0f, 13.0f, 5.0f));
    path.positionKeyFrames.push_back(vec3(-10.0f, 14.0f, 8.0f));
    path.positionKeyFrames.push_back(vec3(-15.0f, 15.0f, 10.0f));

	path.frontKeyFrames.push_back(vec3(0.0f, -1.0f, 0.0f));
    path.frontKeyFrames.push_back(vec3(10.0f, -1.0f, 10.0f));
    path.frontKeyFrames.push_back(vec3(0.0f, -1.0f, 5.0f));
    path.frontKeyFrames.push_back(vec3(-8.0f, -1.0f, 5.0f));
    path.frontKeyFrames.push_back(vec3(10.0f, 1.0f, 10.0f));

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