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
static glmodel* mugModel;
static glshaderprogram* renderModelDebug;

void labscene::setupProgram() {
	//Debug Program/////////////////
	renderModelDebug = new glshaderprogram({"shaders/debug/basictex.vert", "shaders/debug/basictex.frag"});
	////////////////////////////////
}

sceneCamera* labscene::setupCamera() {
	PathDescriptor path;

	path.positionKeyFrames.push_back(vec3(-1.72f, 0.0f, -0.9f));
	path.positionKeyFrames.push_back(vec3(-1.72f, 0.0f, -0.6f));
	path.positionKeyFrames.push_back(vec3(-1.52f, -0.1f, -0.6f));
	path.positionKeyFrames.push_back(vec3(-1.32f, -0.3f, -1.6f));
	path.positionKeyFrames.push_back(vec3(-0.82f, -0.3f, -2.4f));
	path.positionKeyFrames.push_back(vec3(-0.12f, -0.3f, -1.6f));
    
	path.frontKeyFrames.push_back(vec3(-1.72f, -0.35f, -1.9f));
    path.frontKeyFrames.push_back(vec3(-1.72f, -0.3f, -1.6f));
    path.frontKeyFrames.push_back(vec3(-1.52f, -0.4f, -1.6f));
    path.frontKeyFrames.push_back(vec3(-0.82f, -0.4f, -1.6f));
    path.frontKeyFrames.push_back(vec3(-0.82f, -0.4f, -1.6f));
    path.frontKeyFrames.push_back(vec3(1.93f, -0.42f, -0.29f));

	return new sceneCamera(&path);
}

void labscene::init() {
	labModel = new glmodel("resources/models/spaceship/SpaceLab.fbx", 0, false);
	// mugModel = new glmodel("resources/models/mug/mug.fbx", 0, false);
}

void labscene::render() {
	renderModelDebug->use();
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("mMat"), 1, GL_FALSE, mat4::identity());
	labModel->draw(renderModelDebug);
	// glUniformMatrix4fv(renderModelDebug->getUniformLocation("mMat"), 1, GL_FALSE, mat4::identity());
	// mugModel->draw(renderModelDebug);
}

void labscene::uninit() {
	delete labModel;
}