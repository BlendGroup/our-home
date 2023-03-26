#include <bits/types/FILE.h>
#include<iostream>

#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/vmath.h"
#include"../include/rvModel.hpp"
#include"../include/testeffect.h"

using namespace std;

static glprogram_dl testRenderProgram;
static GLuint texture;
static rvModel* model;
GLuint tempVao;

void GLAPIENTRY ErrorCallback(GLenum src, GLenum type, GLuint id, GLenum saverity, GLsizei length, const GLchar* message, const void* userParam);

void setupProgramTestEffect() {
	glshader_dl vertexShader;
	glshader_dl fragmentShader;

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "src/shaders/modelAnimation.vert", DL_SHADER_CORE, 460);
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "src/shaders/modelAnimation.frag", DL_SHADER_CORE, 460);

	//cout<<glprogramCreate(&testRenderProgram, "Render", {vertexShader, fragmentShader});
	testRenderProgram.programObject = glCreateProgram();
	glAttachShader(testRenderProgram.programObject, vertexShader.shaderObject);
	glAttachShader(testRenderProgram.programObject, fragmentShader.shaderObject);
	glBindAttribLocation(testRenderProgram.programObject, 0, "a_position");
    glBindAttribLocation(testRenderProgram.programObject, 1, "a_normal");
    glBindAttribLocation(testRenderProgram.programObject, 2, "a_texcoord");
    glBindAttribLocation(testRenderProgram.programObject, 3, "a_tangent");
    glBindAttribLocation(testRenderProgram.programObject, 4, "a_bitangent");
    glBindAttribLocation(testRenderProgram.programObject, 5, "a_boneIds");
    glBindAttribLocation(testRenderProgram.programObject, 6, "a_weights");
	glLinkProgram(testRenderProgram.programObject);
	GLint linkedStatus;
	glGetProgramiv(testRenderProgram.programObject, GL_LINK_STATUS, &linkedStatus);
	if(!linkedStatus) {
		char buffer[1024];
		glGetProgramInfoLog(testRenderProgram.programObject, 1024, NULL, buffer);
		string s(buffer);
		cout << ": linking failed.\n" + s + "\n";
	}
	model->initShaders(testRenderProgram.programObject);
	//glshaderDestroy(&vertexShader);
	//glshaderDestroy(&fragmentShader);
}

void initTestEffect() {

	model = new rvModel();
	model->loadModel("resources/vampire/dancing_vampire.dae");
	//model->loadAnimation("resources/vampire/Idle.dae");

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

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	static float t = 0.0f;
	t += 0.0065f;
	mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();

	modelMatrix *= vmath::translate(0.0f, 0.0f, -5.0f);
    viewMatrix = vmath::lookat(vec3(0.0f,0.0f,5.0f),vec3(0.0f,0.0f,-1.0f) ,vec3(0.0f,1.0f,0.0f));

	glUseProgram(testRenderProgram.programObject);
	glUniformMatrix4fv(glGetUniformLocation(testRenderProgram.programObject,"u_Model"), 1, GL_FALSE, modelMatrix);
    glUniformMatrix4fv(glGetUniformLocation(testRenderProgram.programObject,"u_View"), 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(glGetUniformLocation(testRenderProgram.programObject,"u_Projection"), 1, GL_FALSE, perspective);
	//glBindVertexArray(tempVao);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	model->draw(testRenderProgram.programObject,t);
    glUseProgram(0);
}

void uninitTestEffect() {
	if(model)
	{
		model->ModelCleanUp();
		model = nullptr;
	}
	glprogramDestory(&testRenderProgram);
}

void GLAPIENTRY ErrorCallback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	FILE *glLog;
    glLog = fopen("GLLOG.txt", "a+");
    fprintf(glLog,"GL CALLBACK: %s type = 0x%x, serverity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
    fclose(glLog);
}