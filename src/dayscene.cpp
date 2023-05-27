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
#include<debugcamera.h>

using namespace std;
using namespace vmath;

enum EVENTS {
	CAMERA_MOVE = 0,
	CROSSFADE,

//Dont Add	
	NUM_EVENTS
};

static bool eventManager[NUM_EVENTS];

static glshaderprogram* terrainRenderer;

static terrain* land;

static debugCamera* tempCam;

static GLfloat crossT		= 0.0f;

void dayscene::setupProgram() {
	try {
		terrainRenderer = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/render.frag"});
	} catch(string errorString)  {
		throwErr(errorString);
	}
}

void dayscene::setupCamera() {
	tempCam = new debugCamera(vec3(0.0f, 0.0f, 5.0f), -90.0f, 0.0f);
}

void dayscene::init() {
	ivec2 dim = ivec2(2048, 2048);
	GLuint valleyHeightMap = opensimplexnoise::createFBMTexture2D(dim, ivec2(0, 0), 1000.0f, 3, 1234);
	GLuint mountainHeightMap = opensimplexnoise::createTurbulenceFBMTexture2D(dim, ivec2(0, 0), 1500.0f, 5, 0.11f, 543);
	GLuint finalTerrain = opensimplexnoise::combineTwoNoiseTextures(mountainHeightMap, valleyHeightMap, dim, 0.0f);
	// GLuint finalTerrain = createTexture2D("resources/textures/heightmap2.png");
	land = new terrain(finalTerrain, true, 5.0f, 32.0f);
}

void dayscene::render() {
	terrainRenderer->use();
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, -30.0f));
	glUniform1f(terrainRenderer->getUniformLocation("maxTess"), land->getMaxTess());
	glUniform1f(terrainRenderer->getUniformLocation("minTess"), land->getMinTess());
	glUniform3fv(terrainRenderer->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(terrainRenderer->getUniformLocation("texHeight"), 0);
	glUniform1i(terrainRenderer->getUniformLocation("texNormal"), 1);
	glUniform1f(terrainRenderer->getUniformLocation("amplitudeMin"), 3.0f);
	glUniform1f(terrainRenderer->getUniformLocation("amplitudeMax"), 60.0f);
	glBindTextureUnit(0, land->getHeightMap());
	glBindTextureUnit(1, land->getNormalMap());
	land->render();
}

void dayscene::update() {
	t += programglobal::deltaTime;

	if(crossT >= 1.0f) {
		eventManager[CROSSFADE] = false;
		eventManager[CAMERA_MOVE] = true;
	}
}

void dayscene::reset() {

}

void dayscene::uninit() {
	delete land;
}

void dayscene::keyboardfunc(int key) {
}

camera* dayscene::getCamera() {
	return tempCam;
}
