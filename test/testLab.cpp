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
static bool crt = true;
static int v = 0;
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
        diffusePass = new glshaderprogram({"shaders/testStatic.vert", "shaders/testStatic.frag"});
        //program->printUniforms(cout);
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

        envMap = new CubeMapRenderTarget(2048,2048,false);
        envMap->setPosition(vec3(0.0f,0.0f,0.0f));

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
/*
        diffusePass->use();
        glUniformMatrix4fv(diffusePass->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(diffusePass->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix());
        glUniformMatrix4fv(diffusePass->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
        static_model[0]->draw(diffusePass,1);
*/
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
                glUniform3fv(programStaticPBR->getUniformLocation("viewPos"),1,envMap->position);
                // Lights data
                glUniform1i(programStaticPBR->getUniformLocation("numOfLights"),lightsLab.size());
                for(int i = 0; i < lightsLab.size(); i++){
                    glUniform3fv(programStaticPBR->getUniformLocation("light["+to_string(i)+"].diffuse"),1,lightsLab[i].diffuse);
                    glUniform3fv(programStaticPBR->getUniformLocation("light["+to_string(i)+"].position"),1,lightsLab[i].position);
                }
                glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
                static_model[0]->draw(programStaticPBR,1);
            }
            glBindFramebuffer(GL_FRAMEBUFFER,0);
            crt = false;
        }

        programStaticPBR->use();
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"),1,GL_FALSE,envMap->projection);
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"),1,GL_FALSE,envMap->view[v]);
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
            glActiveTexture(GL_TEXTURE16);
			glUniform1i(lightProgram->getUniformLocation("envmap"),16);
            glBindTexture(GL_TEXTURE_CUBE_MAP,envMap->cubemap_texture);
            glBindVertexArray(cubeVao);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
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
