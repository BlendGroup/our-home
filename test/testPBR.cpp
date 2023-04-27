#include <assimp/postprocess.h>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <vmath.h>

#include <glshaderloader.h>
#include <gltextureloader.h>
#include <glmodelloader.h>
#include <CubeMapRenderTarget.h>
#include <vmath.h>
#include <errorlog.h>
#include <testPBR.h>
#include <global.h>

#define DYNAMIC 1

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static glshaderprogram* lightProgram;
static glmodel* model;
static CubeMapRenderTarget* envMap;

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
            program = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrSG.frag"});
        #else
            //program = new glshaderprogram({"src/shaders/pbr.vert", "src/shaders/pbr.frag"});
            program = new glshaderprogram({"src/shaders/pbr.vert", "src/shaders/pbrMR.frag"});
        #endif
        lightProgram = new glshaderprogram({"shaders/testStatic.vert", "shaders/point.frag"});
        //program->printUniforms(cout);

        glViewport(0, 0, envMap->width, envMap->height);
        glBindFramebuffer(GL_FRAMEBUFFER,envMap->FBO);
        program->use();
        glUniformMatrix4fv(program->getUniformLocation("pMat"),1,GL_FALSE,envMap->projection);
        glUniformMatrix4fv(program->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(0.1f,0.1f,0.1f));
        glUniform1i(program->getUniformLocation("numOfLights"),lights.size());
        for(int i = 0; i < lights.size(); i++){
            glUniform3fv(program->getUniformLocation("light["+to_string(i)+"].diffuse"),1,lights[i].diffuse);
            glUniform3fv(program->getUniformLocation("light["+to_string(i)+"].position"),1,lights[i].position);
        }
        glUniform3fv(program->getUniformLocation("viewPos"),1,envMap->getPosition());
        for(size_t i = 0; i < 6; i++)
        {
            glUniformMatrix4fv(program->getUniformLocation("vMat"),1,GL_FALSE,envMap->view[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,envMap->cubemap_texture,0); 
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            model->draw(program,1);
        }
        glBindFramebuffer(GL_FRAMEBUFFER,0);

    } catch (string errorString) {
        throwErr(errorString);
    }
}

void initTestPbr(){
    try {       
        #if DYNAMIC  
        model = new glmodel("resources/models/robot/robot.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,true);
        #else
        model = new glmodel("resources/models/spaceship/SpaceLab.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,true);
        
        //model = new glmodel("resources/models/door/door.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_RemoveRedundantMaterials | aiProcess_FlipUVs,true);
        #endif
        envMap = new CubeMapRenderTarget(2048,2048,true);
        envMap->setPosition(vec3(0.0f));

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
        lights.push_back({vec3(100.0f,100.0f,100.0f),vec3(5.0f,  5.0f, 5.0f)});

        // render lab once
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
        glUniformMatrix4fv(program->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
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
            glActiveTexture(GL_TEXTURE16);
			glUniform1i(lightProgram->getUniformLocation("envmap"),16);
			glBindTexture(GL_TEXTURE_CUBE_MAP,envMap->cubemap_texture);
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
