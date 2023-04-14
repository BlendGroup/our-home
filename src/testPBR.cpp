#include <assimp/postprocess.h>
#include <iostream>
#include <vmath.h>

#include "../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include "../include/glmodelloader.h"
#include "../include/vmath.h"
#include "../include/errorlog.h"
#include "../include/testPBR.h"
#include "../include/global.h"

#define DYNAMIC 1

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static glmodel* model;

void setupProgramTestPbr(){

    try 
    {
        #if DYNAMIC
            program = new glshaderprogram({"src/shaders/pbrDynamic.vert", "src/shaders/pbrSG.frag"});
        #else
            //program = new glshaderprogram({"src/shaders/pbr.vert", "src/shaders/pbr.frag"});
            program = new glshaderprogram({"src/shaders/pbr.vert", "src/shaders/pbrSG.frag"});
        #endif
        //program->printUniforms(cout);
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void initTestPbr(){
    try {       
        #if DYNAMIC  
        model = new glmodel("resources/models/robot/robot.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,true);
        #else
        model = new glmodel("resources/models/robot/robot.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_RemoveRedundantMaterials | aiProcess_FlipUVs,true);
        #endif
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void renderTestPbr(camera *cam,vec3 camPos){
    try {
        program->use();
        glUniformMatrix4fv(program->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(program->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix()); 
        #if DYNAMIC
        glUniformMatrix4fv(program->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(0.1f,0.1f,0.1f));
        model->update(0.005f, 0);
        model->setBoneMatrixUniform(program->getUniformLocation("bMat[0]"), 0);
        #else
        glUniformMatrix4fv(program->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(0.1f,0.1f,0.1f));
        #endif
        glUniform3fv(program->getUniformLocation("viewPos"),1,camPos);
        // Lights data
        glUniform1i(program->getUniformLocation("numOfLights"),4);
        glUniform3fv(program->getUniformLocation("light[0].diffuse"),1,vec3(100.0,100.0,100.0));
        glUniform3fv(program->getUniformLocation("light[0].position"),1,vec3(-10.0f,  10.0f, 10.0f));
        glUniform3fv(program->getUniformLocation("light[1].diffuse"),1,vec3(300.0,300.0,300.0));
        glUniform3fv(program->getUniformLocation("light[1].position"),1,vec3(10.0f,  10.0f, 10.0f));
        glUniform3fv(program->getUniformLocation("light[2].diffuse"),1,vec3(300.0,300.0,300.0));
        glUniform3fv(program->getUniformLocation("light[2].position"),1,vec3(-10.0f, 10.0f, -10.0f));
        glUniform3fv(program->getUniformLocation("light[3].diffuse"),1,vec3(300.0,300.0,300.0));
        glUniform3fv(program->getUniformLocation("light[3].position"),1,vec3(10.0f, 10.0f, -10.0f));
        // Material properties       
        glUniform3fv(program->getUniformLocation("material.diffuse"),1,vec3(1.0,1.0,1.0));
        glUniform1f(program->getUniformLocation("material.metallic"),0.1);
        glUniform1f(program->getUniformLocation("material.roughness"),0.1);
        glUniform1f(program->getUniformLocation("material.ao"),0.0);
        // Texture Properties
        glUniform1i(program->getUniformLocation("isTextured"),GL_TRUE);
        model->draw(program,1);
        glBindTexture(GL_TEXTURE_2D, 0);
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void uninitTestPbr(){
    delete program;
    delete model;
}
