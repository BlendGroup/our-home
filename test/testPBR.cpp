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
#include <glLight.h>
#include <testPBR.h>
#include <global.h>

#define DYNAMIC 0

using namespace std;
using namespace vmath;

static glshaderprogram* program;
static glshaderprogram* lightProgram;
static glmodel* model;
static SceneLight* sceneLights;

void setupProgramTestPbr(){
    try 
    {
        #if DYNAMIC
            program = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
        #else
            //program = new glshaderprogram({"src/shaders/pbr.vert", "src/shaders/pbr.frag"});
            program = new glshaderprogram({"shaders/pbr.vert", "shaders/pbrMain.frag"});
        #endif
        lightProgram = new glshaderprogram({"shaders/debug/lightSrc.vert", "shaders/debug/lightSrc.frag"});
        //program->printUniforms(cout);
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void initTestPbr(){
    try {       
        #if DYNAMIC  
        model = new glmodel("resources/models/astronaut/MCAnim.glb",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,true);
        #else
        model = new glmodel("resources/models/spaceship/LabOut.glb",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,true);
        
        //model = new glmodel("resources/models/door/door.fbx",aiProcessPreset_TargetRealtime_Quality | aiProcess_RemoveRedundantMaterials | aiProcess_FlipUVs,true);
        #endif
        // Lighting setup
        sceneLights = new SceneLight();
        //sceneLights->addDirectionalLight(DirectionalLight(vec3(0.1f),10.0f,vec3(0.0,0.0,-1.0f)));
        sceneLights->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(-5.0f,5.0f,-5.0f),15.0f));
        sceneLights->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(5.0f,5.0f,-5.0f),15.0f));
        sceneLights->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(5.0f,5.0f,5.0f),15.0f));
        sceneLights->addPointLight(PointLight(vec3(1.0f,0.0f,1.0f),100.0f,vec3(-5.0f,5.0f,5.0f),15.0f));
        //sceneLights->addSpotLight(SpotLight(vec3(0.0f,1.0f,0.0f),100.0f,vec3(-6.0f,8.0f,3.5f),35.0f,vec3(0.0f,0.0f,-1.0f),30.0f,45.0f));
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
        glUniformMatrix4fv(program->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(0.5f,0.5f,0.5f));
        #endif
        glUniform3fv(program->getUniformLocation("viewPos"),1,camPos);
        // pbr data
        glUniform1i(program->getUniformLocation("specularGloss"),false);
        // Lights data
        sceneLights->setLightUniform(program);
        model->draw(program,1);

        // Redner Light Src
        lightProgram->use();
        glUniformMatrix4fv(lightProgram->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(lightProgram->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix()); 
        sceneLights->renderSceneLights(lightProgram);

    } catch (string errorString) {
        throwErr(errorString);
    }
}

void uninitTestPbr(){
    delete program;
    delete model;
    delete  sceneLights;
}
