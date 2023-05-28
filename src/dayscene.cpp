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
#include<crossfade.h>

using namespace std;
using namespace vmath;

enum EVENTS {
	CAMERA_MOVE = 0,
	CROSSFADE_IN,

//Dont Add	
	NUM_EVENTS
};

static bool eventManager[NUM_EVENTS];

static glshaderprogram* terrainRenderer;

static terrain* land;
static terrain* land2;

static debugCamera* tempCam;

static GLfloat crossinT		= 0.0f;

extern GLuint texLabSceneFinal;

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
GLuint mapMap;
void dayscene::init() {
	for(int i = 0; i < NUM_EVENTS; i++) {
		eventManager[i] = false;
	}
	eventManager[CROSSFADE_IN] = true;

	ivec2 dim = ivec2(2048, 2048);
	GLuint valleyHeightMap = opensimplexnoise::createFBMTexture2D(dim, ivec2(0, 0), 1000.0f, 3, 1234);
	GLuint mountainHeightMap = opensimplexnoise::createTurbulenceFBMTexture2D(dim, ivec2(0, 0), 1500.0f, 7, 0.11f, 543);
	GLuint finalTerrain = opensimplexnoise::combineTwoNoiseTextures(mountainHeightMap, valleyHeightMap, dim, 0.0f);
	// GLuint finalTerrain = createTexture2D("resources/textures/heightmap2.png");
	land = new terrain(valleyHeightMap, true, 5.0f, 32.0f);
	land2 = new terrain(mountainHeightMap, true, 5.0f, 32.0f);

	mapMap = createTexture2D("resources/textures/map.png");
}

void dayscene::render() {
	terrainRenderer->use();
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, -30.0f));
	glUniform1f(terrainRenderer->getUniformLocation("maxTess"), land->getMaxTess());
	glUniform1f(terrainRenderer->getUniformLocation("minTess"), land->getMinTess());
	glUniform3fv(terrainRenderer->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(terrainRenderer->getUniformLocation("texHeight1"), 0);
	glUniform1i(terrainRenderer->getUniformLocation("texNormal1"), 1);
	glUniform1i(terrainRenderer->getUniformLocation("texHeight2"), 2);
	glUniform1i(terrainRenderer->getUniformLocation("texNormal2"), 3);
	glUniform1i(terrainRenderer->getUniformLocation("texMap"), 4);
	glUniform1f(terrainRenderer->getUniformLocation("amplitudeMin"), 15.0f);
	glUniform1f(terrainRenderer->getUniformLocation("amplitudeMax"), 100.0f);
	glBindTextureUnit(0, land->getHeightMap());
	glBindTextureUnit(1, land->getNormalMap());
	glBindTextureUnit(2, land2->getHeightMap());
	glBindTextureUnit(3, land2->getNormalMap());
	glBindTextureUnit(4, mapMap);
	land->render();

	if(eventManager[CROSSFADE_IN]) {
		crossfader::render(texLabSceneFinal, crossinT);
	}
}

void dayscene::update() {
	t += programglobal::deltaTime;

	if(crossinT >= 1.0f) {
		eventManager[CROSSFADE_IN] = false;
		eventManager[CAMERA_MOVE] = true;
	}

	if(eventManager[CROSSFADE_IN]) {
		crossinT += 0.3f * programglobal::deltaTime;
	}
}

void dayscene::reset() {
	t = 0.0f;
}

void dayscene::uninit() {
	delete land;
}

void dayscene::keyboardfunc(int key) {
}

camera* dayscene::getCamera() {
	return tempCam;
}
