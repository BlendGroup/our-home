#include<scenes/day.h>
#include<glmodelloader.h>
#include<glshaderloader.h>
#include<gltextureloader.h>
#include<iostream>
#include<vmath.h>
#include<scenecamera.h>
#include<scenecamerarig.h>
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
#include<crossfade.h>
#include<lake.h>
#include<eventmanager.h>

using namespace std;
using namespace vmath;

static glshaderprogram* programTerrain;
static glshaderprogram* programLake;
static glshaderprogram* programStaticPBR;
static glshaderprogram* programColor;

static terrain* land;
static terrain* land2;
static lake* lake1;

static glmodel* modelLab;
static glmodel* modelRover;
static glmodel* modelTreePine;
static glmodel* modelTreeRed;
static glmodel* modelTreePurple;

static SceneLight* lightManager;

static sceneCamera* camera1;

#ifdef DEBUG
static modelplacer* lakePlacer;
static glshaderprogram* drawTexQuad;
static sceneCameraRig* camRig1;
static bool renderPath = false;
#endif

static GLfloat lakeT = 0.0f;

static GLuint texTerrainMap;
static GLuint texDiffuseGrass;
static GLuint texDiffuseDirt;
static GLuint texDiffuseMountain;
static GLuint texLakeMap;
static GLuint texLakeDuDVMap;
extern GLuint texLabSceneFinal;

enum tvalues {
	CROSSIN_T,
	CAMERAMOVE_T
};
static eventmanager* dayevents;

void dayscene::setupProgram() {
	try {
		programTerrain = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/render.frag"});
		programLake = new glshaderprogram({"shaders/lake/render.vert", "shaders/lake/render.frag"});
		programStaticPBR = new glshaderprogram({"shaders/pbr.vert", "shaders/pbrMain.frag"});
		programColor = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
#ifdef DEBUG
		drawTexQuad = new glshaderprogram({"shaders/debug/basictex.vert", "shaders/debug/basictex.frag"});
#endif
	} catch(string errorString)  {
		throwErr(errorString);
	}
}

void dayscene::setupCamera() {
	vector<vec3> positionVector = {
		vec3(62.1965f, -0.31273f, -77.7934f),
		vec3(47.2794f, 4.3124f, -89.3915f),
		vec3(26.8796f, 9.6124f, -105.691f),
		vec3(-0.720439f, 14.7124f, -116.891f),
		vec3(-36.3205f, 14.7124f, -130.091f),
		vec3(-73.0202f, 11.8124f, -130.791f),
		vec3(-88.8205f, 16.1124f, -123.691f),
		vec3(-84.0206f, 16.1124f, -106.191f)
	};
	vector<vec3> frontVector = {
		vec3(61.8794f, -0.287602f, -82.8909f),
		vec3(36.4794f, 5.61237f, -95.9913f),
		vec3(7.27934f, 12.2124f, -101.591f),
		vec3(-23.5207f, 9.81237f, -113.391f),
		vec3(-52.021f, 1.51237f, -106.091f),
		vec3(-70.3207f, 4.01237f, -105.991f),
		vec3(-81.8206f, 13.7124f, -104.691f),
		vec3(-74.3207f, 15.9124f, -89.1912f)
	};
	camera1 = new sceneCamera(positionVector, frontVector);

	camRig1 = new sceneCameraRig(camera1);
	camRig1->setRenderFront(true);
	camRig1->setRenderFrontPoints(true);
	camRig1->setRenderPath(true);
	camRig1->setRenderPathPoints(true);
	camRig1->setRenderPathToFront(true);
	camRig1->setScalingFactor(0.1f);
}

void dayscene::init() {
	dayevents = new eventmanager({
		{CROSSIN_T, { 0.0f, 2.6f }},
		{CAMERAMOVE_T, { 2.6f, 23.0f }}
	});

	ivec2 dim = ivec2(2048, 2048);
	GLuint valleyHeightMap = opensimplexnoise::createFBMTexture2D(dim, ivec2(0, 0), 900.0f, 3, 1234);
	GLuint mountainHeightMap = opensimplexnoise::createTurbulenceFBMTexture2D(dim, ivec2(0, 0), 1200.0f, 6, 0.11f, 543);
	land = new terrain(valleyHeightMap, 256, true, 5.0f, 16.0f);
	land2 = new terrain(mountainHeightMap, 256, true, 5.0f, 16.0f);
	CLErr(clhelpererr = clFinish(programglobal::oclContext->getCommandQueue()));

	modelLab = new glmodel("resources/models/spaceship/LabOut.glb", aiProcess_FlipUVs, true);
	modelRover = new glmodel("resources/models/rover/rover.glb", aiProcess_FlipUVs, true);
	modelTreePine = new glmodel("resources/models/tree/pine.glb", 0, true);
	modelTreeRed = new glmodel("resources/models/tree/redtree.fbx", 0, true);
	modelTreePurple = new glmodel("resources/models/tree/purpletree.glb", 0, true);

	lightManager = new SceneLight();
	lightManager->addPointLight(PointLight(vec3(1.0f, 1.0f, 1.0f), 1.0f, vec3(0.0f, 100.0f, 0.0f), 2.0f));

	texTerrainMap = createTexture2D("resources/textures/map.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseGrass = createTexture2D("resources/textures/grass.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseDirt = createTexture2D("resources/textures/dirt.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseMountain = createTexture2D("resources/textures/rocks2.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texLakeMap = createTexture2D("resources/textures/lake.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texLakeDuDVMap = createTexture2D("resources/textures/dudv.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);

	lake1 = new lake(-6.0f);

#ifdef DEBUG
	// lakePlacer = new modelplacer(vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 10.0f);
	// vec3(-49.0f, -6.0f, -72.0f), vec3(0.0f, 0.0f, 0.0f), 38.0f -> Lake
	// vec3(53.1005f, -3.23743f, -56.8485f), vec3(0f, 0f, 0f), 0.00910002f -> Rover
	lakePlacer = new modelplacer();
#endif
}

void dayscene::renderScene(bool cameraFlip) {
	programTerrain->use();
	glUniformMatrix4fv(programTerrain->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programTerrain->getUniformLocation("vMat"), 1, GL_FALSE, cameraFlip ? programglobal::currentCamera->matrixYFlippedOnPlane(lake1->getLakeHeight()) : programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, -30.0f));
	glUniform1f(programTerrain->getUniformLocation("maxTess"), land->getMaxTess());
	glUniform1f(programTerrain->getUniformLocation("minTess"), land->getMinTess());
	glUniform3fv(programTerrain->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programTerrain->getUniformLocation("texHeight1"), 0);
	glUniform1i(programTerrain->getUniformLocation("texNormal1"), 1);
	glUniform1i(programTerrain->getUniformLocation("texHeight2"), 2);
	glUniform1i(programTerrain->getUniformLocation("texNormal2"), 3);
	glUniform1i(programTerrain->getUniformLocation("texMap"), 4);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseGrass"), 5);
	// glUniform1i(programTerrain->getUniformLocation("texDiffuseDirt"), 6);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseMountain"), 7);
	glUniform1i(programTerrain->getUniformLocation("texLake"), 8);
	glUniform1f(programTerrain->getUniformLocation("amplitudeMin"), 10.0f);
	glUniform1f(programTerrain->getUniformLocation("amplitudeMax"), 100.0f);
	glUniform1f(programTerrain->getUniformLocation("texScale"), 10.0f);
	glUniform1f(programTerrain->getUniformLocation("lakeDepth"), 10.0f);
	glUniform1f(programTerrain->getUniformLocation("clipy"), lake1->getLakeHeight());
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
	// programColor->use();
	// mat4 mvp = 
	// 	programglobal::perspective * 
	// 	cameraFlip ? programglobal::currentCamera->matrixYFlippedOnPlane(lake1->getLakeHeight()) : programglobal::currentCamera->matrix() *
	// 	lakePlacer->getModelMatrix();
	// glUniformMatrix4fv(programColor->getUniformLocation("mvpMatrix"), 1, GL_FALSE, mvp);
	// glUniform4f(programColor->getUniformLocation("color"), 0.8f, 0.3f, 0.1f, 1.0f);
	// glUniform4f(programColor->getUniformLocation("emissive"), 0.8f, 0.3f, 0.1f, 1.0f);
	// glUniform4f(programColor->getUniformLocation("occlusion"), 0.0f, 0.0f, 0.0f, 1.0f);
	// programglobal::shapeRenderer->renderSphere();
}

void dayscene::render() {
	camera1->setT((*dayevents)[CAMERAMOVE_T]);

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

	programStaticPBR->use();
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniform3fv(programStaticPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programStaticPBR->getUniformLocation("specularGloss"), GL_FALSE);
	lightManager->setLightUniform(programStaticPBR, false);

	glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(61.8599f, -6.7f, -69.4705f) * scale(0.87f));
	modelLab->draw(programStaticPBR);

	glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(53.1005f, -3.23743f, -56.8485f) * scale(0.00910002f));
	modelRover->draw(programStaticPBR);
	
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(61.0f, 6.7f, -53.4f) * scale(0.7f));
	modelTreePine->draw(programStaticPBR);
	
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(54.89f, 2.2f, -45.3f) * scale(2.9f));
	modelTreeRed->draw(programStaticPBR);

	glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(48.0f, -5.0f, -34.0f) * scale(2.0f));
	modelTreePurple->draw(programStaticPBR);

	programLake->use();
	glUniformMatrix4fv(programLake->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programLake->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programLake->getUniformLocation("mMat"), 1, GL_FALSE, translate(-49.0f, -6.0f, -72.0f) * scale(38.0f));
	glUniform1i(programLake->getUniformLocation("texRefraction"), 0);
	glUniform1i(programLake->getUniformLocation("texReflection"), 1);
	glUniform1i(programLake->getUniformLocation("texDuDv"), 2);
	glUniform1f(programLake->getUniformLocation("time"), lakeT);
	glUniform3fv(programLake->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glBindTextureUnit(0, lake1->getRefractionTexture());
	glBindTextureUnit(1, lake1->getReflectionTexture());
	glBindTextureUnit(2, texLakeDuDVMap);
	lake1->render();

	if(programglobal::debugMode == CAMERA) {
		camRig1->render();
	}

	// glDisable(GL_DEPTH_TEST);
	// drawTexQuad->use();
	// glUniformMatrix4fv(drawTexQuad->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	// glUniformMatrix4fv(drawTexQuad->getUniformLocation("vMat"), 1, GL_FALSE, mat4::identity());
	// glUniformMatrix4fv(drawTexQuad->getUniformLocation("mMat"), 1, GL_FALSE, translate(-0.36f, 0.181f, -0.7f) * scale(0.1f));
	// glUniform1i(drawTexQuad->getUniformLocation("texture_diffuse"), currentTex);
	// glBindTextureUnit(0, lake1->getReflectionTexture());
	// glBindTextureUnit(1, lake1->getRefractionTexture());
	// glBindTextureUnit(2, lake1->getDepthTexture());
	// programglobal::shapeRenderer->renderQuad();
	// glEnable(GL_DEPTH_TEST);
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
	} else if(programglobal::debugMode == CAMERA) {
		camRig1->keyboardfunc(key);
	}
	switch(key) {
	case XK_Up:
		(*dayevents) += 0.4f;
		break;
	case XK_Down:
		(*dayevents) -= 0.4f;
	case XK_F2:
		renderPath = !renderPath;
		camRig1->setRenderPathToFront(renderPath);
		break;
	case XK_Tab:
		if(programglobal::debugMode == CAMERA) {
			cout<<camRig1->getCamera()<<endl;
		} else 
		// if(programglobal::debugMode == SPLINE) {
		// 	cout<<robotSpline->getSpline()<<endl;
		// }
		if(programglobal::debugMode == MODEL) {
			cout<<lakePlacer<<endl;
		}
		break;
	}
}

camera* dayscene::getCamera() {
	return camera1;
}

void dayscene::crossfade() {
	if((*dayevents)[CROSSIN_T] < 1.0f) {
		crossfader::render(texLabSceneFinal, (*dayevents)[CROSSIN_T]);
	}
}