#include <bits/types/FILE.h>
#include<iostream>

#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/vmath.h"
#include"../include/rvModel.hpp"
#include"../include/testeffect.h"

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static GLuint texture;
static rvModel* model;
GLuint tempVao;

void setupProgramTestEffect() {
	program = new glshaderprogram({"src/shaders/modelAnimation.vert", "src/shaders/modelAnimation.frag"});
	model->initShaders(program);
}

void initTestEffect() {

	model = new rvModel();
	model->loadModel("resources/vampire.fbx");
	//model->loadAnimation("resources/Vampire/Dancing.dae");
}

void renderTestEffect(mat4 perspective) {
	static float t = 0.0f;
	t += 0.0065f;
	mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();

	modelMatrix *= vmath::translate(0.0f, 0.0f, -5.0f);
    viewMatrix = vmath::lookat(vec3(0.0f,0.0f,5.0f),vec3(0.0f,0.0f,-1.0f) ,vec3(0.0f,1.0f,0.0f));

	program->use();
	glUniformMatrix4fv(program->getUniformLocation("u_Model"), 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(program->getUniformLocation("u_View"), 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(program->getUniformLocation("u_Projection"), 1, GL_FALSE, perspective);
	//glBindVertexArray(tempVao);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	model->draw(program, t);
    glUseProgram(0);
}

void uninitTestEffect() {
	if(model)
	{
		model->ModelCleanUp();
		model = nullptr;
	}
	delete program;
}
