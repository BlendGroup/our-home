#include<iostream>
#include<assimp/postprocess.h>

#include"../include/glshaderloader.h"
#include"../include/glmodelloader.h"
#include"../include/vmath.h"
#include"../include/errorlog.h"

#include"../include/testeffect.h"

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static GLuint texture;
static glmodel* model;

#define DYNAMIC 1

void setupProgramTestModel() {
	try {
#if DYNAMIC
		program = new glshaderprogram({"src/shaders/testDynamic.vert", "src/shaders/testDynamic.frag"});
#else
		program = new glshaderprogram({"src/shaders/testStatic.vert", "src/shaders/testStatic.frag"});
#endif
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void initTestModel() {
	try {
#if DYNAMIC
		model = new glmodel("resources/models/vampire/dancing_vampire.dae", aiProcess_FlipUVs);
#else
		model = new glmodel("resources/models/backpack/backpack.obj", 0);
#endif
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void renderTestModel() {
	try {
		program->use();
		glUniformMatrix4fv(program->getUniformLocation("pMat"), 1, GL_FALSE, perspective(45.0f, 1.0f, 0.1f, 100.0f));
		glUniformMatrix4fv(program->getUniformLocation("vMat"), 1, GL_FALSE, lookat(vec3(0.0f, 0.0f, 6.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)));
#if DYNAMIC
		glUniformMatrix4fv(program->getUniformLocation("mMat"), 1, GL_FALSE, rotate(0.0f, vec3(0.0f, 1.0f, 0.0f)) * translate(0.0f, -0.7f, 0.0f) * scale(1.5f));
		model->update(0.01f, 0);
		model->setBoneMatrixUniform(program->getUniformLocation("bMat[0]"), 0);
#else
		glUniformMatrix4fv(program->getUniformLocation("mMat"), 1, GL_FALSE, rotate(0.0f, vec3(0.0f, 1.0f, 0.0f)));
#endif
		model->draw();
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void uninitTestModel() {
	delete program;
}