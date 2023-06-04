#include "glshaderloader.h"
#include "vmath.h"
#include <glmodelloader.h>
#include <assimp/postprocess.h>
#include <debugcamera.h>
#include <skyrender.h>
#include <cmath>
#include <iostream>

SkyRenderSystem::SkyRenderSystem(){
    sphereMesh = new glmodel("resources/models/sphere.glb",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,false);
    skyShader = new glshaderprogram({"shaders/AtmosphericScattering/Sky.vert","shaders/AtmosphericScattering/Sky.frag"});

    innerRadius = 10.0f * 10.0f;
    outerRadius = 12.25f * 10.0f;

    g = -0.95f;
    Kr = 0.0025f;
    Km = 0.001f;
    sun = 10.0f;
    waveLength4[0] = powf(0.650f, 4.0f);
    waveLength4[1] = powf(0.570f, 4.0f);
    waveLength4[2] = powf(0.475f, 4.0f);
    animateSun = true;
    sunDir = vmath::vec3(0.0f,0.017f,-1.0f);
    sunAngle = vmath::radians(1.0f);
}

SkyRenderSystem::~SkyRenderSystem(){

    if(sphereMesh)
        delete sphereMesh;
    
    if(skyShader)
        delete skyShader;
}

void SkyRenderSystem::Render(vmath::mat4 pMat, camera *cam, float dt){

    try{

        //glDisable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        //glDepthMask(GL_FALSE);
        vmath::vec3 camPos = cam->position();
        vmath::mat4 model = vmath::translate(camPos[0],camPos[1]-innerRadius,camPos[2]) * vmath::scale(vmath::vec3(outerRadius));
        //model = vmath::inverse(model) * vmath::scale(vmath::vec3(outerRadius));
        skyShader->use();
        glUniformMatrix4fv(skyShader->getUniformLocation("pMat"),1,GL_FALSE,pMat);
        glUniformMatrix4fv(skyShader->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix());
        glUniformMatrix4fv(skyShader->getUniformLocation("mMat"),1,GL_FALSE,model);
        glUniform3f(skyShader->getUniformLocation("cameraPos"),0.0f,innerRadius,0.0f);
        glUniform3f(skyShader->getUniformLocation("lightDir"),sunDir[0],sunDir[1],sunDir[2]);
        glUniform1f(skyShader->getUniformLocation("g"), g);
        glUniform3f(skyShader->getUniformLocation("invWaveLength4"), 1.0f / waveLength4[0], 1.0f / waveLength4[1], 1.0f / waveLength4[2]);
        glUniform1f(skyShader->getUniformLocation("innerRadius"),innerRadius);
        glUniform1f(skyShader->getUniformLocation("outerRadius"),outerRadius);
        glUniform1f(skyShader->getUniformLocation("Kr"),Kr);
        glUniform1f(skyShader->getUniformLocation("Km"),Km);
        glUniform1f(skyShader->getUniformLocation("sun"),sun);
        glUniform1f(skyShader->getUniformLocation("Scale"),1.0f/(outerRadius - innerRadius));
        sphereMesh->draw(skyShader,1,false);

        if(animateSun){
            double pp = M_PI + vmath::radians(20.0f);
            sunAngle = (sunAngle + 0.5 * dt) - (M_PI + vmath::radians(20.0f)) * std::floor((sunAngle + 0.5 * dt) / (M_PI + vmath::radians(20.0f)));
            sunDir[1] = sinf(sunAngle);
            sunDir[2] = -cosf(sunAngle);
        }

        //glDepthMask(GL_TRUE);
        //glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
    
    }catch(std::string errorString)
    {
        throwErr(errorString);
    }
}
