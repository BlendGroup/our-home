#include <debugcamera.h>
#include <glmodelloader.h>
#include <vmath.h>
#include <assimp/postprocess.h>
#include <atmosphere.h>
#include <cmath>
#include <iostream>
#include <X11/keysym.h>
#include <global.h>

using namespace std;
using namespace vmath;

static debugCamera* staticcam;

Atmosphere::Atmosphere()
{
    this->SetDefaults();
    sphereModel = new glmodel("resources/models/sphere.glb",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,false);
    atmosphereProgram = new glshaderprogram({"shaders/AtmosphericScattering/Atmosphere.vert","shaders/AtmosphericScattering/Atmosphere.frag"});
    staticcam = new debugCamera(vec3(0.0f, 0.0f, 0.0f), 270.0f, 20.0f);
}

Atmosphere::~Atmosphere(){
    if(sphereModel)
        delete sphereModel;
    
    if(atmosphereProgram)
        delete  atmosphereProgram;

    if(staticcam)
        delete staticcam;
}

void Atmosphere::SetDefaults(){

    viewSamples = defViewSamples;
    lightSamples = defLightSamples;

    SetSunDefaults();
    setEarthRadius(e_R_e);
    setAtmosRadius(e_R_a);
    beta_R = e_beta_R;
    beta_M = e_beta_M;

    H_R = e_H_R;
    H_M = e_H_M;
    g = e_g;
}

void Atmosphere::SetSunDefaults(){
    animateSun = true;
    sunDir = defSunDir;
    setSunAngle(defSunAngle);
    I_Sun = e_I_sun;
}

void Atmosphere::SetRayLightDefaults(){
    beta_R = e_beta_R;
    H_R = e_H_R;
}

void Atmosphere::SetMieDefaults(){
    beta_M = e_beta_M;
    H_M = e_H_M;
    g = e_g;
}

void Atmosphere::SetsizeDefaults(){
    setEarthRadius(e_R_e);
    setAtmosRadius(e_R_a);
}

float keyangle = 0.0f;

void Atmosphere::render(const mat4& viewMatrix, float dt){

    try{
            // 1 Set Properties of the atmosphere
        modelAtmos = translate(0.0f, -6260.0f, 0.0f) * rotate(-134.0f, 0.0f, 1.0f, 0.0f) * scale(6420.0f);

        atmosphereProgram->use();
        glUniformMatrix4fv(atmosphereProgram->getUniformLocation("M"),1,GL_FALSE, scale(R_a));
        glUniformMatrix4fv(atmosphereProgram->getUniformLocation("mMat"),1,GL_FALSE, modelAtmos);
        glUniformMatrix4fv(atmosphereProgram->getUniformLocation("pMat"),1,GL_FALSE, programglobal::perspective);
        glUniformMatrix4fv(atmosphereProgram->getUniformLocation("vMat"),1,GL_FALSE, staticcam->matrix() * viewMatrix);
        glUniform3fv(atmosphereProgram->getUniformLocation("viewPos"), 1, vec3(0.0f, R_e, 30.0f));
        glUniform1i(atmosphereProgram->getUniformLocation("viewSamples"), viewSamples);
        glUniform1i(atmosphereProgram->getUniformLocation("lightSamples"), lightSamples);

        glUniform1f(atmosphereProgram->getUniformLocation("I_sun"), I_Sun);
        glUniform1f(atmosphereProgram->getUniformLocation("R_e"), R_e);
        glUniform1f(atmosphereProgram->getUniformLocation("R_a"), R_a);
        glUniform3fv(atmosphereProgram->getUniformLocation("beta_R"),1, beta_R);
        glUniform1f(atmosphereProgram->getUniformLocation("beta_M"), beta_M);
        glUniform1f(atmosphereProgram->getUniformLocation("H_R"), H_R);
        glUniform1f(atmosphereProgram->getUniformLocation("H_M"), H_M);
        glUniform1f(atmosphereProgram->getUniformLocation("g"), g);
        glUniform3fv(atmosphereProgram->getUniformLocation("sunPos"), 1, sunDir);
        sphereModel->draw(atmosphereProgram,1,false);
        if(animateSun){
            sunAngle = dt;
            sunDir[1] = sinf(sunAngle);
            sunDir[2] = -cosf(sunAngle);
        }
    }
    catch(string errorString){
        throwErr(errorString);
    }
}

void Atmosphere::keyboardfunc(int key) {
	switch(key) {
	case XK_period:
		keyangle += 1.0f;
		break;
	case XK_comma:
		keyangle -= 1.0f;
		break;
	}
	cout<<keyangle<<endl;
}