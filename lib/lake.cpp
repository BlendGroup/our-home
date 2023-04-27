#include<lake.h>
#include<GL/glew.h>
#include<GL/gl.h>
#include<glshaderloader.h>
#include<global.h>
#include<vmath.h>
#include<iostream>

using namespace std;
using namespace vmath;

lake::lake(mat4 modelMat) {
	this->modelMatrix = modelMat;
}

void lake::setupProgram(void) {
	this->renderLake = new glshaderprogram({"shaders/lake/render.vert", "shaders/lake/render.frag"});
	// this->renderLake->printUniforms(cout);
}

void lake::init(void) {
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);
}

void lake::render(void) {
	this->renderLake->use();
	glUniformMatrix4fv(this->renderLake->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(this->renderLake->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(this->renderLake->getUniformLocation("mMat"), 1, GL_FALSE, this->modelMatrix);
	glBindVertexArray(this->vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(vao);
}

void lake::uninit(void) {
	glDeleteVertexArrays(1, &this->vao);
}

lake::~lake() {
	delete this->renderLake;
}