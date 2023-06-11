#include <debugcamera.h>
#include <glmodelloader.h>
#include <vmath.h>
#include <assimp/postprocess.h>
#include <atmosphere.h>
#include <cmath>
#include <iostream>
#include <X11/keysym.h>
#include <global.h>
#include <sphere.h>

using namespace std;
using namespace vmath;

static GLuint vao;
static GLuint eabo;

static sphere* sphereObj;
static mat4 localViewMat;
static const int defViewSamples = 16;
static const int defLightSamples = 8;
static const float e_I_sun = 20.f;
static const float e_R_e = 6360.00;
static const float e_R_a = 6420.00;
static const vec3 e_beta_R = vec3(0.0058f, 0.0135f, 0.0331f);
static const float e_beta_M = 0.0210f;
static const float e_H_R = 7.994;
static const float e_H_M = 1.200;
static const float e_g = 0.888;

Atmosphere::Atmosphere() {
	sphereObj = new sphere(25, 25, 1.0f);
	atmosphereProgram = new glshaderprogram({"shaders/AtmosphericScattering/Atmosphere.vert","shaders/AtmosphericScattering/Atmosphere.frag"});
	debugCamera cam(vec3(0.0f, 0.0f, 0.0f), 270.0f, 20.0f);
	localViewMat = cam.matrix();
}

Atmosphere::~Atmosphere() {
	if(sphereObj)
		delete sphereObj;
	
	if(atmosphereProgram)
		delete  atmosphereProgram;
}

void Atmosphere::render(const mat4& viewMatrix, float sunAngle){
	try{
		mat4 modelAtmos = translate(0.0f, -6260.0f, 0.0f) * rotate(-134.0f, 0.0f, 1.0f, 0.0f) * scale(6420.0f);

		atmosphereProgram->use();
		glUniformMatrix4fv(atmosphereProgram->getUniformLocation("M"),1,GL_FALSE, scale(e_R_a));
		glUniformMatrix4fv(atmosphereProgram->getUniformLocation("mMat"),1,GL_FALSE, modelAtmos);
		glUniformMatrix4fv(atmosphereProgram->getUniformLocation("pMat"),1,GL_FALSE, programglobal::perspective);
		glUniformMatrix4fv(atmosphereProgram->getUniformLocation("vMat"),1,GL_FALSE, localViewMat * viewMatrix);
		glUniform3fv(atmosphereProgram->getUniformLocation("viewPos"), 1, vec3(0.0f, e_R_e, 30.0f));
		glUniform1i(atmosphereProgram->getUniformLocation("viewSamples"), defViewSamples);
		glUniform1i(atmosphereProgram->getUniformLocation("lightSamples"), defLightSamples);
		glUniform1f(atmosphereProgram->getUniformLocation("I_sun"), e_I_sun);
		glUniform1f(atmosphereProgram->getUniformLocation("R_e"), e_R_e);
		glUniform1f(atmosphereProgram->getUniformLocation("R_a"), e_R_a);
		glUniform3fv(atmosphereProgram->getUniformLocation("beta_R"),1, e_beta_R);
		glUniform1f(atmosphereProgram->getUniformLocation("beta_M"), e_beta_M);
		glUniform1f(atmosphereProgram->getUniformLocation("H_R"), e_H_R);
		glUniform1f(atmosphereProgram->getUniformLocation("H_M"), e_H_M);
		glUniform1f(atmosphereProgram->getUniformLocation("g"), e_g);
		glUniform3fv(atmosphereProgram->getUniformLocation("sunPos"), 1, vec3(0.0f, sinf(sunAngle), -cosf(sunAngle)));
		sphereObj->render();
	}
	catch(string errorString){
		throwErr(errorString);
	}
}

void Atmosphere::keyboardfunc(int key) {
	switch(key) {
	case XK_period:
		break;
	case XK_comma:
		break;
	}
}