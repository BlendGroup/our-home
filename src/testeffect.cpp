#include<iostream>

#include<glshaderloader.h>
#include<gltextureloader.h>
#include<vmath.h>
#include<errorlog.h>
#include<testeffect.h>

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static GLuint texture;

void setupProgramTestEffect() {
	try {
		program = new glshaderprogram({"src/shaders/point.vert", "src/shaders/point.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void initTestEffect() {
	try {
		glPointSize(480.0f);

		GLuint tempVao;
		glGenVertexArrays(1, &tempVao);
		glBindVertexArray(tempVao);

		texture = createTexture2D("resources/textures/demo.png", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void renderTestEffect() {
	try {
		program->use();
		glUniform1i(program->getUniformLocation("texSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_POINTS, 0, 1);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void uninitTestEffect() {
	delete program;
}