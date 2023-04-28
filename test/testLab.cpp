#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <vector>
#include <vmath.h>

#include <glshaderloader.h>
#include <gltextureloader.h>
#include <glmodelloader.h>
#include <vmath.h>
#include <errorlog.h>
#include <testLab.h>
#include <global.h>
#include <glLight.h>
#include <CubeMapRenderTarget.h>
#include<X11/keysym.h>

using namespace std;
using namespace vmath;

static glshaderprogram* programStaticPBR,*programDynamicPBR;
static glshaderprogram* lightProgram;
static glshaderprogram* diffusePass;
static vector<glmodel*> static_model;
static vector<glmodel*> dynamic_model;
static CubeMapRenderTarget* envMap;
static SceneLight* sceneLights;
static bool crt = true;
static int v = 0;

void setupProgramTestLab(){
    try 
    {
        programStaticPBR = new glshaderprogram({"shaders/pbr.vert", "shaders/pbrMain.frag"});
        programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
        lightProgram = new glshaderprogram({"shaders/debug/lightSrc.vert", "shaders/debug/lightSrc.frag"});
        diffusePass = new glshaderprogram({"shaders/pbr.vert", "shaders/common.frag"});
        //program->printUniforms(cout);
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void initTestLab(){
    try {

        static_model.push_back(new glmodel("resources/models/spaceship/SpaceLab.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs, true));
        dynamic_model.push_back(new glmodel("resources/models/robot/robot.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,true));
        envMap = new CubeMapRenderTarget(2048,2048,false);
        envMap->setPosition(vec3(0.0f,0.0f,0.0f));

        // Lighting setup
        sceneLights = new SceneLight();
        sceneLights->addDirectionalLight(DirectionalLight(vec3(0.1f),10.0f,vec3(0.0,0.0,-1.0f)));
        sceneLights->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(-5.0f,5.0f,-5.0f),25.0f));
        sceneLights->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(5.0f,5.0f,-5.0f),25.0f));
        sceneLights->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(5.0f,5.0f,5.0f),25.0f));
        sceneLights->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(-5.0f,5.0f,5.0f),25.0f));
        //sceneLights->addSpotLight(SpotLight(vec3(0.0f,1.0f,0.0f),100.0f,vec3(-6.0f,8.0f,3.5f),35.0f,vec3(0.0f,0.0f,-1.0f),30.0f,45.0f));

    } catch (string errorString) {
        throwErr(errorString);
    }
}

void renderTestLab(camera *cam,vec3 camPos){
    try {
        // Static Objects Pass

        if(crt){
            glBindFramebuffer(GL_FRAMEBUFFER,envMap->FBO);
			glViewport(0, 0, envMap->width, envMap->height);

            for(int side = 0; side < 6; side++){
                std::cout<<"i "<<side<<std::endl;
                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + side,envMap->cubemap_texture,0); 
                glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.1f, 0.1f, 1.0f));
                glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));

                programStaticPBR->use();
                glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"),1,GL_FALSE,envMap->projection);
                glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"),1,GL_FALSE,envMap->view[side]);
                glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
                glUniform3fv(programStaticPBR->getUniformLocation("viewPos"),1,envMap->position);
                // Lights data
                glUniform1i(programStaticPBR->getUniformLocation("specularGloss"),false);
                sceneLights->setLightUniform(programStaticPBR);
                static_model[0]->draw(programStaticPBR,1);
            }
            glBindFramebuffer(GL_FRAMEBUFFER,0);
            crt = false;
        }

        programStaticPBR->use();
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix());
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
        glUniform3fv(programStaticPBR->getUniformLocation("viewPos"),1,camPos);
        // Lights data
        glUniform1i(programStaticPBR->getUniformLocation("specularGloss"),false);
        sceneLights->setLightUniform(programStaticPBR);
        static_model[0]->draw(programStaticPBR,1);    

        // Dynamic Objects Pass
        programDynamicPBR->use();
        glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix());
        glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"),1,GL_FALSE,translate(-3.0f,-0.15f,-1.0f) * scale(0.05f,0.05f,0.05f));
        dynamic_model[0]->update(0.005f, 0);
        dynamic_model[0]->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
        glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"),1,camPos);
        // Lights data
        glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"),true);
        sceneLights->setLightUniform(programDynamicPBR);
        dynamic_model[0]->update(0.005f, 0);
        dynamic_model[0]->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
        dynamic_model[0]->draw(programDynamicPBR,1); 

        // render light src
        lightProgram->use();
        glUniformMatrix4fv(lightProgram->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(lightProgram->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix()); 
        sceneLights->renderSceneLights(lightProgram);
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void keyboardFuncTestLab(int key) {
	switch(key) {
	case XK_V: case XK_v:
            ++v;
            if(v > 5)
                v = 0;
		break;
	}
}

void uninitTestLab(){
    delete programStaticPBR;
    delete programDynamicPBR;
    delete diffusePass;
    delete lightProgram;
    static_model.clear();
    dynamic_model.clear();
}
