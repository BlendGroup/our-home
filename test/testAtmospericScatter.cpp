#include "camera.h"
#include <X11/X.h>
#include <assimp/postprocess.h>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <vmath.h>

#include <glshaderloader.h>
#include <gltextureloader.h>
#include <glmodelloader.h>
#include <vmath.h>
#include <errorlog.h>
#include <atmosphere.h>
#include <testAtmospericScatter.h>
#include <global.h>

using namespace std;
using namespace vmath;

static glmodel* sun;
static Atmosphere* atmosphere;
static glshaderprogram* color,*skybox;
static GLuint skybox_vao,vbo;

void setupProgramAS(){
    try 
    {
        color = new glshaderprogram({"shaders/color.vert","shaders/color.frag"});
        skybox = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/debug/rendercubemap.frag"});
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void initAS(){
    try {
        sun = new glmodel("resources/models/sphere.glb",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,false);
        atmosphere = new Atmosphere(sun);

        float skybox_positions[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
        };
        glCreateVertexArrays(1, &skybox_vao);
        glBindVertexArray(skybox_vao);
        glCreateBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_positions), skybox_positions, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);

    } catch (string errorString) {
        throwErr(errorString);
    }
}

void renderAS(camera *cam,vec3 camPos){
    try {
        static float dt = 0.0f;

        //skybox->use();
		//glUniformMatrix4fv(skybox->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
		//glUniformMatrix4fv(skybox->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix());
		//glBindVertexArray(skybox_vao);
		//glBindTextureUnit(0,atmosphere->getAtmosphereTexture());
		//glDrawArrays(GL_TRIANGLES, 0, 36);

        color->use();
        glUniformMatrix4fv(color->getUniformLocation("mvpMatrix"),1,GL_FALSE,programglobal::perspective * cam->matrix() * translate(0.0f,0.0f,0.0f));
        glUniform4fv(color->getUniformLocation("color"),1,vec4(1.0f,0.0f,0.0f,1.0f));
        sun->draw(color,1,false);
        
        atmosphere->setProjView(programglobal::perspective, cam->matrix());
        atmosphere->setViewPos(camPos);
        atmosphere->render(dt);
        dt += 0.00001f;

        //glDepthMask(GL_TRUE);
        // render to cubemap test
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void uninitAS(){

    if(atmosphere)
        delete atmosphere;
    if(sun)
        delete sun;
    if(color)
        delete color;
    if(skybox)
        delete skybox;
}
