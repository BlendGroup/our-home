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
#include "../include/testPBR.h"
#include "../include/global.h"

#define DYNAMIC 1

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static glshaderprogram* lightProgram;
static glmodel* model;

GLuint lightVao;

struct Light{
    vec3 diffuse;
    vec3 position;
};

vector<Light>lights;

void setupProgramTestPbr(){
    try 
    {
        #if DYNAMIC
            program = new glshaderprogram({"src/shaders/pbrDynamic.vert", "src/shaders/pbrSG.frag"});
        #else
            //program = new glshaderprogram({"src/shaders/pbr.vert", "src/shaders/pbr.frag"});
            program = new glshaderprogram({"src/shaders/pbr.vert", "src/shaders/pbrSG.frag"});
        #endif
        lightProgram = new glshaderprogram({"src/shaders/testStatic.vert", "src/shaders/point.frag"});
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
        
        //model = new glmodel("resources/models/door/door.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_RemoveRedundantMaterials | aiProcess_FlipUVs,true);
        #endif
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
        glCreateVertexArrays(1, &lightVao);
        glBindVertexArray(lightVao);
        GLuint vbo;
        glCreateBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        lights.push_back({vec3(100.0f,100.0f,100.0f),vec3(5.0f,  5.0f, 5.0f)});
        lights.push_back({vec3(100.0f,100.0f,100.0f),vec3(-5.0f,  5.0f, 5.0f)});
        lights.push_back({vec3(100.0f,100.0f,100.0f),vec3(-5.0f, 5.0f, -5.0f)});
        lights.push_back({vec3(100.0f,100.0f,100.0f),vec3(5.0f, 5.0f, -5.0f)});
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
        glUniformMatrix4fv(program->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) *scale(1.0f,1.0f,1.0f));
        #endif
        glUniform3fv(program->getUniformLocation("viewPos"),1,camPos);
        // Lights data
        glUniform1i(program->getUniformLocation("numOfLights"),lights.size());
        for(int i = 0; i < lights.size(); i++){
            glUniform3fv(program->getUniformLocation("light["+to_string(i)+"].diffuse"),1,lights[i].diffuse);
            glUniform3fv(program->getUniformLocation("light["+to_string(i)+"].position"),1,lights[i].position);
        }
        model->draw(program,1);
        glBindTexture(GL_TEXTURE_2D, 0);

        for(auto l : lights){
            lightProgram->use();
            glUniformMatrix4fv(lightProgram->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
            glUniformMatrix4fv(lightProgram->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix()); 
            glUniformMatrix4fv(lightProgram->getUniformLocation("mMat"),1,GL_FALSE,translate(l.position) * scale(1.0f,1.0f,1.0f));
            glUniform3fv(lightProgram->getUniformLocation("color"),1,l.diffuse);
            glBindVertexArray(lightVao);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void uninitTestPbr(){
    delete program;
    delete model;
}
