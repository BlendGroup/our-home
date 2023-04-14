#include<iostream>
#include<assimp/postprocess.h>
#include<glshaderloader.h>
#include<glmodelloader.h>
#include<vmath.h>
#include<errorlog.h>
#include<testmodel.h>
#include<global.h>

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static GLuint texture;
static glmodel* model;
//static model* m;

#define DYNAMIC 1

void setupProgramTestModel() {
	try {
#if DYNAMIC
		program = new glshaderprogram({"src/shaders/testDynamic.vert", "src/shaders/testDynamic.frag"});
#else
		program = new glshaderprogram({"src/shaders/testStatic.vert", "src/shaders/testStatic.frag"});
#endif
	// program->printUniforms(cerr);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void initTestModel() {
	try {
#if DYNAMIC
		model = new glmodel("resources/models/vampire/vamp.fbx", aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,false);
#else
		model = new glmodel("resources/models/sphere/sphere.obj", aiProcessPreset_TargetRealtime_Quality,true);
		//m = new model("resources/models/vampire/dancing_vampire.dae");
#endif
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void renderTestModel(camera* cam) {
	try {
		program->use();
		glUniformMatrix4fv(program->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
		glUniformMatrix4fv(program->getUniformLocation("vMat"), 1, GL_FALSE, cam->matrix());
#if DYNAMIC
		glUniformMatrix4fv(program->getUniformLocation("mMat"), 1, GL_FALSE, rotate(0.0f, vec3(0.0f, 1.0f, 0.0f)) * translate(0.0f, -0.7f, 0.0f) * scale(1.5f));
		model->update(0.005f, 1,2,0.5f);
		model->setBoneMatrixUniform(program->getUniformLocation("bMat[0]"), 1);
#else
		glUniformMatrix4fv(program->getUniformLocation("mMat"), 1, GL_FALSE, rotate(0.0f, vec3(0.0f, 1.0f, 0.0f)));
#endif
		model->draw(program);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void uninitTestModel() {
	delete program;
}