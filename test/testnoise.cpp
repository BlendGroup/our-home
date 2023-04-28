#include<iostream>

#include<glshaderloader.h>
#include<gltextureloader.h>
#include<vmath.h>
#include<errorlog.h>
#include<testnoise.h>
#include<global.h>

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static GLuint texture;
static GLuint tempVao;
	
void setupProgramTestNoise() {
	try {
		program = new glshaderprogram({"shaders/point.vert", "shaders/point.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void initTestNoise() {
	try {
		glPointSize(480.0f);

		glGenVertexArrays(1, &tempVao);
		glBindVertexArray(tempVao);

		texture = programglobal::noiseGenerator->createNoiseTextureOnUniformInput(Noise2D, ivec2(1024, 1024), ivec2(0, 0), 128.0f, 1.0f, 3423);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void renderTestNoise() {
	try {
		program->use();
		glUniform1i(program->getUniformLocation("texSampler"), 0);
		glBindVertexArray(tempVao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_POINTS, 0, 1);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void uninitTestNoise() {
	delete program;
}