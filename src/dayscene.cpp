#define DEBUG
#include<scenes/day.h>
#include<glmodelloader.h>
#include<glshaderloader.h>
#include<gltextureloader.h>
#include<iostream>
#include<vmath.h>
#include<scenecamera.h>
#include<modelplacer.h>
#include<X11/keysym.h>
#include<interpolators.h>
#include<splinerenderer.h>
#include<CubeMapRenderTarget.h>
#include<terrain.h>
#include<glLight.h>
#include<assimp/postprocess.h>
#include<global.h>
#include<opensimplexnoise.h>

using namespace std;
using namespace vmath;

glshaderprogram* terrainRenderer;

static terrain* valley;
static GLuint valleyHeightMap;
static GLuint heightMapMap;

void dayscene::setupProgram() {
	try {
		terrainRenderer = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/render.frag"});
	} catch(string errorString)  {
		throwErr(errorString);
	}
}

sceneCamera* dayscene::setupCamera() {
	vector<vec3> positionKeyFrames = {
		vec3(-1.96f, -0.22f, -1.11f),
		vec3(-1.82f, -0.08f, -0.91f),
		vec3(-1.71f, -0.12f, -1.07f),
		vec3(-1.63f, -0.3f, -1.7f),
		vec3(-1.15f, -0.05f, -1.81f),
		vec3(-0.82f, 0.28f, -1.85f),
		vec3(-1.17f, 0.06f, -1.1f),
		vec3(-2.9f, -0.2f, -0.51f),
		vec3(-3.49f, -0.56f, 0.05f)
	};
    
	vector<vec3> frontKeyFrames = {
		vec3(-1.96f, -0.35f, -1.65f),
		vec3(-1.74f, -0.3f, -1.29f),
		vec3(-1.38f, -0.47f, -1.48f),
		vec3(-1.26f, -0.4f, -1.5f),
		vec3(-0.8f, -0.28f, -1.23f),
		vec3(-0.47f, -0.26f, -0.37f),
		vec3(-1.67f, -0.42f, 0.45f),
		vec3(-3.19f, -0.36f, 0.96f),
		vec3(-3.49f, -0.27f, 2.02f)
	};

	return new sceneCamera(positionKeyFrames, frontKeyFrames);
}

void dayscene::init() {
	valleyHeightMap = opensimplexnoise::createFBMTexture2D(ivec2(2048, 2048), ivec2(0, 0), 500.0f, 2, 1234);
	valley = new terrain(valleyHeightMap);
	heightMapMap = createTexture2D("resources/textures/heightmapmap.png");
}

void dayscene::render() {
	terrainRenderer->use();
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, -30.0f));
	glUniform1i(terrainRenderer->getUniformLocation("numMeshes"), MESH_SIZE);
	glUniform1f(terrainRenderer->getUniformLocation("maxTess"), MAX_PATCH_TESS_LEVEL);
	glUniform1f(terrainRenderer->getUniformLocation("minTess"), MIN_PATCH_TESS_LEVEL);
	glUniform3fv(terrainRenderer->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(terrainRenderer->getUniformLocation("texHeight"), 0);
	glUniform1i(terrainRenderer->getUniformLocation("texNormal"), 1);
	glUniform1f(terrainRenderer->getUniformLocation("amplitude"), 1.0f);
	glBindTextureUnit(0, valley->getHeightMap());
	glBindTextureUnit(1, valley->getNormalMap());
	valley->render();
}

void dayscene::uninit() {
	delete valley;
}

void dayscene::keyboardfunc(int key) {
}
