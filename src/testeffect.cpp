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

void GLAPIENTRY ErrorCallback(GLenum src, GLenum type, GLuint id, GLenum saverity, GLsizei length, const GLchar* message, const void* userParam);

void setupProgramTestEffect() {
	program = new glshaderprogram({"src/shaders/modelAnimation.vert", "src/shaders/modelAnimation.frag"});
	model->initShaders(program);
}

void initTestEffect() {

	model = new rvModel();
	model->loadModel("resources/vampire.fbx");
	//model->loadAnimation("resources/Vampire/Dancing.dae");

    glClearColor(0.0f,0.25f,0.25f,1.0f);

    // Depth Related Changes
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(ErrorCallback,0);
	//glPointSize(480.0f);
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

void GLAPIENTRY ErrorCallback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	FILE *glLog;
    glLog = fopen("GLLOG.txt", "a+");
    fprintf(glLog,"GL CALLBACK: %s type = 0x%x, serverity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
    fclose(glLog);
}

void GLAPIENTRY ErrorCallback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	FILE *glLog;
    glLog = fopen("GLLOG.txt", "a+");
    fprintf(glLog,"GL CALLBACK: %s type = 0x%x, serverity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
    fclose(glLog);
}