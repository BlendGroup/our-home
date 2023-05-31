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
#include<lake.h>
#include<eventmanager.h>

using namespace std;
using namespace vmath;

static glshaderprogram* terrainRenderer;
static glshaderprogram* lakeRenderer;

static terrain* land;
static terrain* land2;
static lake* lake1;

static debugCamera* tempCam;

#ifdef DEBUG
static modelplacer* lakePlacer;
static glshaderprogram* drawTexQuad;
#endif

static GLfloat lakeT = 0.0f;

static GLuint texTerrainMap;
static GLuint texDiffuseGrass;
static GLuint texDiffuseDirt;
static GLuint texDiffuseMountain;
static GLuint texLakeMap;
static GLuint texLakeDuDVMap;
extern GLuint texLabSceneFinal;

static int currentTex = 0;

enum tvalues {
	CROSSIN_T,
	CAMERAMOVE_T
};
static eventmanager* dayevents;

void dayscene::setupProgram() {
	try {
		terrainRenderer = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/render.frag"});
		lakeRenderer = new glshaderprogram({"shaders/lake/render.vert", "shaders/lake/render.frag"});
		drawTexQuad = new glshaderprogram({"shaders/debug/basictex.vert", "shaders/debug/basictex.frag"});
	} catch(string errorString)  {
		throwErr(errorString);
	}
}

void dayscene::setupCamera() {
	tempCam = new debugCamera(vec3(0.0f, 0.0f, 5.0f), -90.0f, 0.0f);
}

void dayscene::init() {
	dayevents = new eventmanager({
		{CROSSIN_T, { 0.0f, 4.0f }}
	});

	ivec2 dim = ivec2(2048, 2048);
	GLuint valleyHeightMap = opensimplexnoise::createFBMTexture2D(dim, ivec2(0, 0), 900.0f, 3, 1234);
	GLuint mountainHeightMap = opensimplexnoise::createTurbulenceFBMTexture2D(dim, ivec2(0, 0), 1200.0f, 6, 0.11f, 543);
	land = new terrain(valleyHeightMap, 256, true, 5.0f, 16.0f);
	land2 = new terrain(mountainHeightMap, 256, true, 5.0f, 16.0f);

	texTerrainMap = createTexture2D("resources/textures/map.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseGrass = createTexture2D("resources/textures/grass.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseDirt = createTexture2D("resources/textures/dirt.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseMountain = createTexture2D("resources/textures/rocks2.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texLakeMap = createTexture2D("resources/textures/lake.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texLakeDuDVMap = createTexture2D("resources/textures/dudv.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);

	lake1 = new lake(-6.0f);

#ifdef DEBUG
	// lakePlacer = new modelplacer(vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 10.0f);
	lakePlacer = new modelplacer(vec3(-49.0f, -6.0f, -72.0f), vec3(0.0f, 0.0f, 0.0f), 38.0f);
#endif
}

void dayscene::renderScene(bool cameraFlip) {
	terrainRenderer->use();
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("vMat"), 1, GL_FALSE, cameraFlip ? programglobal::currentCamera->matrixYFlippedOnPlane(lake1->getLakeHeight()) : programglobal::currentCamera->matrix());
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, -30.0f));
	glUniform1f(terrainRenderer->getUniformLocation("maxTess"), land->getMaxTess());
	glUniform1f(terrainRenderer->getUniformLocation("minTess"), land->getMinTess());
	glUniform3fv(terrainRenderer->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(terrainRenderer->getUniformLocation("texHeight1"), 0);
	glUniform1i(terrainRenderer->getUniformLocation("texNormal1"), 1);
	glUniform1i(terrainRenderer->getUniformLocation("texHeight2"), 2);
	glUniform1i(terrainRenderer->getUniformLocation("texNormal2"), 3);
	glUniform1i(terrainRenderer->getUniformLocation("texMap"), 4);
	glUniform1i(terrainRenderer->getUniformLocation("texDiffuseGrass"), 5);
	// glUniform1i(terrainRenderer->getUniformLocation("texDiffuseDirt"), 6);
	glUniform1i(terrainRenderer->getUniformLocation("texDiffuseMountain"), 7);
	glUniform1i(terrainRenderer->getUniformLocation("texLake"), 8);
	glUniform1f(terrainRenderer->getUniformLocation("amplitudeMin"), 10.0f);
	glUniform1f(terrainRenderer->getUniformLocation("amplitudeMax"), 100.0f);
	glUniform1f(terrainRenderer->getUniformLocation("texScale"), 10.0f);
	glUniform1f(terrainRenderer->getUniformLocation("lakeDepth"), 10.0f);
	glUniform1f(terrainRenderer->getUniformLocation("clipy"), lake1->getLakeHeight());
	glBindTextureUnit(0, land->getHeightMap());
	glBindTextureUnit(1, land->getNormalMap());
	glBindTextureUnit(2, land2->getHeightMap());
	glBindTextureUnit(3, land2->getNormalMap());
	glBindTextureUnit(4, texTerrainMap);
	glBindTextureUnit(5, texDiffuseGrass);
	glBindTextureUnit(6, texDiffuseDirt);
	glBindTextureUnit(7, texDiffuseMountain);
	glBindTextureUnit(8, texLakeMap);
	land->render();
}

void dayscene::render() {
	float distance = 2.0f * (programglobal::currentCamera->position()[1] - lake1->getLakeHeight());
	glEnable(GL_CLIP_DISTANCE0);
	lake1->setReflectionFBO();
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	this->renderScene(true);
	glDisable(GL_CLIP_DISTANCE0);

	glEnable(GL_CLIP_DISTANCE1);
	lake1->setRefractionFBO();
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	this->renderScene();
	glDisable(GL_CLIP_DISTANCE1);

	resetFBO();

	this->renderScene();

	lakeRenderer->use();
	glUniformMatrix4fv(lakeRenderer->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(lakeRenderer->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(lakeRenderer->getUniformLocation("mMat"), 1, GL_FALSE, lakePlacer->getModelMatrix());
	glUniform1i(lakeRenderer->getUniformLocation("texRefraction"), 0);
	glUniform1i(lakeRenderer->getUniformLocation("texReflection"), 1);
	glUniform1i(lakeRenderer->getUniformLocation("texDuDv"), 2);
	glUniform1f(lakeRenderer->getUniformLocation("time"), lakeT);
	glUniform3fv(lakeRenderer->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glBindTextureUnit(0, lake1->getRefractionTexture());
	glBindTextureUnit(1, lake1->getReflectionTexture());
	glBindTextureUnit(2, texLakeDuDVMap);
	lake1->render();

	glDisable(GL_DEPTH_TEST);
	drawTexQuad->use();
	glUniformMatrix4fv(drawTexQuad->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(drawTexQuad->getUniformLocation("vMat"), 1, GL_FALSE, mat4::identity());
	glUniformMatrix4fv(drawTexQuad->getUniformLocation("mMat"), 1, GL_FALSE, translate(-0.36f, 0.181f, -0.7f) * scale(0.1f));
	glUniform1i(drawTexQuad->getUniformLocation("texture_diffuse"), currentTex);
	glBindTextureUnit(0, lake1->getReflectionTexture());
	glBindTextureUnit(1, lake1->getRefractionTexture());
	glBindTextureUnit(2, lake1->getDepthTexture());

	programglobal::shapeRenderer->renderQuad();
	glEnable(GL_DEPTH_TEST);
}

void dayscene::update() {
	dayevents->increment();
	const float LAKE_SPEED = 0.05f;
	lakeT += programglobal::deltaTime * LAKE_SPEED;
}

void dayscene::reset() {
	dayevents->resetT();
}

void dayscene::uninit() {
	delete land;
}

void dayscene::keyboardfunc(int key) {
	if(programglobal::debugMode == MODEL) {
		lakePlacer->keyboardfunc(key);
	}
	switch(key) {
	case XK_Tab:
		//cout << doorPlacer;
		// if(programglobal::debugMode == CAMERA) {
		// 	cout<<cameraRig->getCamera()<<endl;
		// }	
		// if(programglobal::debugMode == SPLINE) {
		// 	cout<<robotSpline->getSpline()<<endl;
		// }
		if(programglobal::debugMode == MODEL) {
			cout<<lakePlacer<<endl;
		}
		break;
	case XK_1: case XK_2: case XK_3:
		currentTex = key - XK_1;
	}
}

camera* dayscene::getCamera() {
	return tempCam;
}

void dayscene::crossfade() {
	if((*dayevents)[CROSSIN_T] < 1.0f) {
		crossfader::render(texLabSceneFinal, (*dayevents)[CROSSIN_T]);
	}
}