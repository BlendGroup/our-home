#include<iostream>

#include"../include/glshaderloader.h"

#include"../include/testeffect.h"

using namespace std;

static glprogram_dl testRenderProgram;

void setupProgramTestEffect() {
	glshader_dl vertexShader;
	glshader_dl fragmentShader;

	cout<<glshaderCreate(&vertexShader, GL_VERTEX_SHADER, "src/shaders/point.vert");
	cout<<glshaderCreate(&fragmentShader, GL_FRAGMENT_SHADER, "src/shaders/point.frag");

	cout<<glprogramCreate(&testRenderProgram, "Render", {vertexShader, fragmentShader});

	glshaderDestroy(&vertexShader);
	glshaderDestroy(&fragmentShader);
}

void initTestEffect() {
	glPointSize(10.0f);

	GLuint tempVao;
	glGenVertexArrays(1, &tempVao);
	glBindVertexArray(tempVao);
}

void renderTestEffect() {
	glUseProgram(testRenderProgram.programObject);
	glDrawArrays(GL_POINTS, 0, 1);
}

void uninitTestEffect() {
	glprogramDestory(&testRenderProgram);
}