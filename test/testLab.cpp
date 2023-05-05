#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <vector>
#include <vmath.h>

#include "../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include "../include/glmodelloader.h"
#include "../include/vmath.h"
#include "../include/errorlog.h"
#include "../include/testLab.h"
#include "../include/global.h"

#define DYNAMIC 0

using namespace std;
using namespace vmath;

static glshaderprogram* programStaticPBR,*programDynamicPBR;
static glshaderprogram* lightProgram;
static vector<glmodel*> static_model;
static vector<glmodel*> dynamic_model;

GLuint cubeVao;

struct Light{
    vec3 diffuse;
    vec3 position;
};

vector<Light>lightsLab;

void setupProgramTestLab(){
    try 
    {
        programStaticPBR = new glshaderprogram({"shaders/pbr.vert", "shaders/pbrMR.frag"});
        programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrSG.frag"});
        lightProgram = new glshaderprogram({"shaders/testStatic.vert", "shaders/point.frag"});
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void initTestLab(){
    try {

        static_model.push_back(new glmodel("resources/models/spaceship/SpaceLab.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs, true));
        dynamic_model.push_back(new glmodel("resources/models/robot/robot.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,true));

        const float cubeVerts[] = {
            -0.5f, -0.5f, -0.5f, 
            0.5f, -0.5f, -0.5f, 
            0.5f,  0.5f, -0.5f, 
            0.5f,  0.5f, -0.5f, 
            -0.5f,  0.5f, -0.5f, 
            -0.5f, -0.5f, -0.5f, 
            -0.5f, -0.5f, 0.5f,  
            0.5f, -0.5f, 0.5f,  
            0.5f,  0.5f, 0.5f,  
            0.5f,  0.5f, 0.5f, 
            -0.5f,  0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,
                                                
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
                                                
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
                                                
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,
                                                
            -0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f,  0.5f,
            0.5f, 0.5f,  0.5f,
            -0.5f, 0.5f,  0.5f,
            -0.5f, 0.5f, -0.5f
        };
        glCreateVertexArrays(1, &cubeVao);
        glBindVertexArray(cubeVao);
        GLuint vbo;
        glCreateBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        lightsLab.push_back({vec3(100.0f,100.0f,100.0f),vec3(5.0f,  5.0f, 5.0f)});
        lightsLab.push_back({vec3(100.0f,100.0f,100.0f),vec3(-5.0f,  5.0f, 5.0f)});
        lightsLab.push_back({vec3(100.0f,100.0f,100.0f),vec3(-5.0f, 5.0f, -5.0f)});
        lightsLab.push_back({vec3(100.0f,100.0f,100.0f),vec3(5.0f, 5.0f, -5.0f)});
        lightsLab.push_back({vec3(100.0f,100.0f,100.0f),vec3(5.0f,  5.0f, 5.0f)});
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void renderTestLab(camera *cam,vec3 camPos){
    try {

        // Static Objects Pass
        programStaticPBR->use();
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix());
        glUniform3fv(programStaticPBR->getUniformLocation("viewPos"),1,camPos);
        // Lights data
        glUniform1i(programStaticPBR->getUniformLocation("numOfLights"),lightsLab.size());
        for(int i = 0; i < lightsLab.size(); i++){
            glUniform3fv(programStaticPBR->getUniformLocation("light["+to_string(i)+"].diffuse"),1,lightsLab[i].diffuse);
            glUniform3fv(programStaticPBR->getUniformLocation("light["+to_string(i)+"].position"),1,lightsLab[i].position);
        }
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
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
        glUniform1i(programDynamicPBR->getUniformLocation("numOfLights"),lightsLab.size());
        for(int i = 0; i < lightsLab.size(); i++){
            glUniform3fv(programDynamicPBR->getUniformLocation("light["+to_string(i)+"].diffuse"),1,lightsLab[i].diffuse);
            glUniform3fv(programDynamicPBR->getUniformLocation("light["+to_string(i)+"].position"),1,lightsLab[i].position);
        }
        dynamic_model[0]->update(0.005f, 0);
        dynamic_model[0]->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
        dynamic_model[0]->draw(programDynamicPBR,1); 

        // render lights for refereance
        for(auto l : lightsLab){
            lightProgram->use();
            glUniformMatrix4fv(lightProgram->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
            glUniformMatrix4fv(lightProgram->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix()); 
            glUniformMatrix4fv(lightProgram->getUniformLocation("mMat"),1,GL_FALSE,translate(l.position) * scale(1.0f,1.0f,1.0f));
            glUniform3fv(lightProgram->getUniformLocation("color"),1,l.diffuse);
            glBindVertexArray(cubeVao);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void uninitTestLab(){
    delete programStaticPBR;
    delete programDynamicPBR;
    static_model.clear();
    dynamic_model.clear();
}
