#include<iostream>

#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/vmath.h"

#include"../include/testeffect.h"

using namespace std;

static glshaderprogram* program;
static GLuint texture;

void setupProgramTestEffect() {
	try {
		program = new glshaderprogram({"src/shaders/point.vert", "src/shaders/point.frag"});
	} catch(string errorString) {
		cout<<errorString<<endl;
	}
}

void initTestEffect() {
	glPointSize(480.0f);

	GLuint tempVao;
	glGenVertexArrays(1, &tempVao);
	glBindVertexArray(tempVao);

	createTexture2D(texture, "resources/textures/demo.png", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

void renderTestEffect() {
	try {
		program->use();
		glUniform1i(program->getUniformLocation("texSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_POINTS, 0, 1);
	} catch(string errorString) {
		cout<<errorString<<endl;
	}
}

void uninitTestEffect() {
	delete program;
}