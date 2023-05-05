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
	vector<vec3> positionKeyFrames;
	positionKeyFrames.push_back(vec3(-1.72f, 0.0f, -0.9f));
	positionKeyFrames.push_back(vec3(-1.72f, 0.0f, -0.6f));
	positionKeyFrames.push_back(vec3(-1.52f, -0.1f, -0.6f));
	positionKeyFrames.push_back(vec3(-1.32f, -0.3f, -1.6f));
	positionKeyFrames.push_back(vec3(-0.82f, -0.3f, -2.4f));
	positionKeyFrames.push_back(vec3(-0.12f, -0.3f, -1.6f));
    
	vector<vec3> frontKeyFrames;
	frontKeyFrames.push_back(vec3(-1.72f, -0.35f, -1.9f));
    frontKeyFrames.push_back(vec3(-1.72f, -0.3f, -1.6f));
    frontKeyFrames.push_back(vec3(-1.52f, -0.4f, -1.6f));
    frontKeyFrames.push_back(vec3(-0.82f, -0.4f, -1.6f));
    frontKeyFrames.push_back(vec3(-0.82f, -0.4f, -1.6f));
    frontKeyFrames.push_back(vec3(1.93f, -0.42f, -0.29f));

	return new sceneCamera(positionKeyFrames, frontKeyFrames);
}

void labscene::init() {
	labModel = new glmodel("resources/models/spaceship/SpaceLab.fbx", 0, false);
	mugModel = new glmodel("resources/models/mug/mug.glb", 0, false);
}

void labscene::render() {
	renderModelDebug->use();
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("mMat"), 1, GL_FALSE, mat4::identity());
	labModel->draw(renderModelDebug);
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("mMat"), 1, GL_FALSE, translate(-1.3f,-0.41f,-1.5f) * scale(0.08f,0.08f,0.08f));
	mugModel->draw(renderModelDebug);
}

void labscene::uninit() {
	delete labModel;
	delete mugModel;
}