#include <iostream>

#include "../include/glshaderloader.h"
#include "../include/glmodelloader.h"
#include "../include/vmath.h"
#include "../include/errorlog.h"
#include "../include/testPBR.h"
#include "../include/global.h"

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static glmodel* model;

void setupProgramTestPbr(){

    try {
        program = new glshaderprogram({"src/shaders/pbr.vert", "src/shaders/pbr.frag"});
        //program->printUniforms(cout);
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void initTestPbr(){
    try {
        model = new glmodel("resources/models/sphere.obj",0);
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void renderTestPbr(camera *cam,vec3 camPos){
    try {
        program->use();
        glUniformMatrix4fv(program->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(program->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix()); 
        glUniformMatrix4fv(program->getUniformLocation("mMat"),1,GL_FALSE,rotate(0.0f, vec3(0.0f, 1.0f, 0.0f)));
        glUniform3fv(program->getUniformLocation("viewPos"),1,camPos);
        // Lights data
        glUniform1i(program->getUniformLocation("numOfLights"),4);
        glUniform3fv(program->getUniformLocation("light[0].diffuse"),1,vec3(300.0,300.0,300.0));
        glUniform3fv(program->getUniformLocation("light[0].position"),1,vec3(-10.0f,  10.0f, 10.0f));
        glUniform3fv(program->getUniformLocation("light[1].diffuse"),1,vec3(300.0,300.0,300.0));
        glUniform3fv(program->getUniformLocation("light[1].position"),1,vec3(10.0f,  10.0f, 10.0f));
        glUniform3fv(program->getUniformLocation("light[2].diffuse"),1,vec3(300.0,300.0,300.0));
        glUniform3fv(program->getUniformLocation("light[2].position"),1,vec3(-10.0f, -10.0f, 10.0f));
        glUniform3fv(program->getUniformLocation("light[3].diffuse"),1,vec3(300.0,300.0,300.0));
        glUniform3fv(program->getUniformLocation("light[3].position"),1,vec3(10.0f, -10.0f, 10.0f));
        // Material properties       
        glUniform3fv(program->getUniformLocation("material.diffuse"),1,vec3(0.5,0.0,0.0));
        glUniform1f(program->getUniformLocation("material.metallic"),0.5);
        glUniform1f(program->getUniformLocation("material.roughness"),0.5);
        glUniform1f(program->getUniformLocation("material.ao"),1.0);
        model->draw();
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void uninitTestPbr(){
    delete program;
    delete model;
}
