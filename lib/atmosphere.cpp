#include "debugcamera.h"
#include <atmosphere.h>
#include <cmath>
#include <iostream>

Atmosphere::Atmosphere(glmodel* tsphereModel)
:sphereModel(tsphereModel)
{
    this->SetDefaults();
    atmosphereProgram = new glshaderprogram({"shaders/AtmosphericScattering/Atmosphere.vert","shaders/AtmosphericScattering/Atmosphere.frag"});
    debugcamera = new debugCamera(vmath::vec3(0.0f, 6359.0f, 30.0f), 270.0f, 20.0f);
    envMapper = new CubeMapRenderTarget(2048, 2048, false);
	envMapper->setPosition(vmath::vec3(0.0f, 6359.0f, 30.0f));
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

void Atmosphere::render(float dt){

    try{

        //glBindFramebuffer(GL_FRAMEBUFFER,envMapper->FBO);
        //glViewport(0, 0, envMapper->width, envMapper->height);

        //for(int side = 0; side < 6; side++)
        {
            //glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + side,envMapper->cubemap_texture,0); 
            //glClearBufferfv(GL_COLOR, 0, vmath::vec4(0.1f, 0.1f, 0.1f, 1.0f));
            //glClearBufferfv(GL_DEPTH, 0, vmath::vec1(1.0f));

            // 1 Set Properties of the atmosphere
            atmosphereProgram->use();
            glUniformMatrix4fv(atmosphereProgram->getUniformLocation("M"),1,GL_FALSE,modelAtmos);
            glUniformMatrix4fv(atmosphereProgram->getUniformLocation("MVP"),1,GL_FALSE,proj * debugcamera->matrix() * modelAtmos);

            glUniform3fv(atmosphereProgram->getUniformLocation("viewPos"),1,vmath::vec3(0.0f, 6359.0f, 30.0f));
            glUniform1i(atmosphereProgram->getUniformLocation("viewSamples"),viewSamples);
            glUniform1i(atmosphereProgram->getUniformLocation("lightSamples"),lightSamples);

            glUniform1f(atmosphereProgram->getUniformLocation("I_sun"),I_Sun);
            glUniform1f(atmosphereProgram->getUniformLocation("R_e"),R_e);
            glUniform1f(atmosphereProgram->getUniformLocation("R_a"),R_a);
            glUniform3fv(atmosphereProgram->getUniformLocation("beta_R"),1,beta_R);
            glUniform1f(atmosphereProgram->getUniformLocation("beta_M"),beta_M);
            glUniform1f(atmosphereProgram->getUniformLocation("H_R"),H_R);
            glUniform1f(atmosphereProgram->getUniformLocation("H_M"),H_M);
            glUniform1f(atmosphereProgram->getUniformLocation("g"),g);
            glUniform3fv(atmosphereProgram->getUniformLocation("sunPos"),1,sunDir);
            sphereModel->draw(atmosphereProgram,1,false);
        }

        //glBindFramebuffer(GL_FRAMEBUFFER,0);

        if(animateSun){
            double pp = M_PI + vmath::radians(20.0f);
            sunAngle = (sunAngle + 0.5 * dt) - (M_PI + vmath::radians(20.0f)) * std::floor((sunAngle + 0.5 * dt) / (M_PI + vmath::radians(20.0f)));
            sunDir[1] = sinf(sunAngle);
            sunDir[2] = -cosf(sunAngle);
        }
    }
    catch(std::string errorString){
        throwErr(errorString);
    }
}
