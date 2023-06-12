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
#include<splineadjuster.h>
#include<audio.h>
#include<godrays.h>
#include<atmosphere.h>

using namespace std;
using namespace vmath;

static glshaderprogram* programTerrain;
static glshaderprogram* programLake;
static glshaderprogram* programStaticPBR;
static glshaderprogram* programDynamicPBR;
static glshaderprogram* programColor;
static glshaderprogram* programCombineMap;
static glshaderprogram* programLight;

static terrain* land;
static terrain* land2;
static lake* lake1;

static glmodel* modelLab;
static glmodel* modelRover;
static glmodel* modelTreePine;
static glmodel* modelTreeRed;
static glmodel* modelTreePurple;
static glmodel* modelDrone;

static SceneLight* lightManager;

static sceneCamera* camera1;
static sceneCamera* camera2;

static BsplineInterpolator* splineDrone;
static BsplineInterpolator* splineRover;

static godrays* godraysDrone;

static Atmosphere* atmosphere;

#ifdef DEBUG
static modelplacer* lakePlacer;
static glshaderprogram* drawTexQuad;
static glshaderprogram* programDrawOnTerrain;
static sceneCameraRig* camRig1;
static sceneCameraRig* camRig2;
static SplineAdjuster* splineAdjuster;
static bool renderPath = false;
static audioplayer* playerBkgnd;
#endif

static GLfloat lakeT = 0.0f;

static GLuint texTerrainMap;
static GLuint texDiffuseGrass;
static GLuint texDiffuseDirt;
static GLuint texDiffuseMountain;
static GLuint texLakeDuDvMap;
extern GLuint texLabSceneFinal;
static GLuint texTerrainHeight;
GLuint texDaySceneFinal;
	
enum tvalues {
	CROSSIN_T,
	GODRAYIN_T,
	CAMERA1MOVE_T,
	CAMERA2MOVE_T,
	DRONETURN_T,
	DRONEMOVE_T,
	SUNRISEINIT_T,
	SUNRISEMID_T,
	SUNRISEEND_T,
	ROVERMOVE_T
};
static eventmanager* dayevents;

void dayscene::setupProgram() {
	try {
		programTerrain = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/rendervalley.frag"});
		programLake = new glshaderprogram({"shaders/lake/render.vert", "shaders/lake/render.frag"});
		programStaticPBR = new glshaderprogram({"shaders/pbrStatic.vert", "shaders/pbrMain.frag"});
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
		programColor = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
		programLight = new glshaderprogram({"shaders/debug/lightSrc.vert", "shaders/debug/lightSrc.frag"});
#ifdef DEBUG
		programDrawOnTerrain = new glshaderprogram({"shaders/debug/pbrOnTerrain.vert", "shaders/pbrMain.frag"});
		drawTexQuad = new glshaderprogram({"shaders/debug/basictex.vert", "shaders/debug/basictex.frag"});
#endif
	} catch(string errorString)  {
		throwErr(errorString);
	}
}

void dayscene::setupCamera() {
	vector<vec3> positionVector = {
		vec3(64.1364f, -0.71273f, -77.6934f),
		vec3(75.3362f, -0.71273f, -81.4933f),
		vec3(89.0362f, 1.78727f, -77.9933f),
		vec3(91.436f, 7.68726f, -61.8939f),
		vec3(91.636f, 9.18727f, -43.6942f),
		vec3(93.7359f, 13.9873f, -21.6943f),
		vec3(94.8359f, 13.9873f, 3.30575f),
		vec3(67.4364f, 7.58727f, -10.5942f),
		vec3(53.2366f, 7.58727f, -22.4943f),
		vec3(51.4967f, 0.98727f, -60.6936f),
		vec3(52.0967f, 1.88727f, -89.4932f),
		vec3(26.7796f, 11.2124f, -105.591f),
		vec3(-0.720439f, 14.7124f, -116.891f),
		vec3(-36.3205f, 14.7124f, -130.091f),
		vec3(-71.8202f, 16.6124f, -130.991f),
		vec3(-82.7206f, 16.1124f, -122.591f),
		vec3(-86.9206f, 18.4124f, -109.391f)
	};
	vector<vec3> frontVector = {
		vec3(64.4991f, -0.167602f, -80.1903f),
		vec3(75.4994f, -1.1676f, -79.8903f),
		vec3(77.7994f, -1.3676f, -70.8904f),
		vec3(83.2992f, 1.5324f, -62.2905f),
		vec3(83.8992f, 2.9324f, -49.6911f),
		vec3(82.3993f, 7.4324f, -32.9914f),
		vec3(90.4992f, 11.6324f, -12.7913f),
		vec3(79.0994f, 9.7324f, -6.29131f),
		vec3(60.1997f, 7.3324f, -18.9914f),
		vec3(51.2998f, 6.1324f, -30.1914f),
		vec3(49.4998f, 2.8324f, -47.9911f),
		vec3(51.3998f, 2.2324f, -59.1909f),
		vec3(47.4796f, 1.3124f, -92.8907f),
		vec3(15.4794f, 7.81237f, -90.7913f),
		vec3(1.47934f, 5.21239f, -93.7911f),
		vec3(-20.0207f, 3.11237f, -102.891f),
		vec3(-29.7207f, 3.11237f, -107.491f),
		vec3(-52.021f, 1.51237f, -106.091f),
		vec3(-67.3207f, 4.01237f, -105.091f),
		vec3(-71.5208f, 10.3124f, -102.791f),
		vec3(-74.3207f, 15.9124f, -89.1912f)
	}; 
	camera1 = new sceneCamera(positionVector, frontVector);

	camRig1 = new sceneCameraRig(camera1);
	camRig1->setRenderFront(true);
	camRig1->setRenderFrontPoints(true);
	camRig1->setRenderPath(true);
	camRig1->setRenderPathPoints(true);
	camRig1->setRenderPathToFront(renderPath);
	camRig1->setScalingFactor(0.1f);

	vector<vec3> positionVector2 = {
		vec3(92.9367f, 9.78723f, -66.594f),
		vec3(12.4962f, -4.11273f, -56.9932f),
		vec3(-12.303f, -4.1127f, -29.093f),
		vec3(-58.2204f, -3.5876f, -47.591f),
		vec3(-36.3204f, -5.78759f, -84.7909f),
		vec3(-3.1203f, 6.31239f, -85.7919f),
		vec3(27.7798f, 33.8124f, -123.192f),
		vec3(49.6795f, 5.41234f, -78.9927f),
		vec3(52.1794f, 35.2124f, 23.2078f)
	};
	vector<vec3> frontVector2 = {
		vec3(36.8991f, 4.1324f, -64.0905f),
		vec3(-11.0205f, -1.6876f, -58.5908f),
		vec3(-24.1207f, -2.8876f, -52.491f),
		vec3(-34.7207f, -4.1876f, -59.091f),
		vec3(2.97928f, 0.712391f, -59.7912f),
		vec3(26.5794f, -1.98761f, -62.4918f),
		vec3(47.3792f, -0.887606f, -58.5918f),
		vec3(54.8792f, 51.5122f, 64.4077f)
	};
	camera2 = new sceneCamera(positionVector2, frontVector2);

	camRig2 = new sceneCameraRig(camera2);
	camRig2->setRenderFront(true);
	camRig2->setRenderFrontPoints(true);
	camRig2->setRenderPath(true);
	camRig2->setRenderPathPoints(true);
	camRig2->setRenderPathToFront(renderPath);
	camRig2->setScalingFactor(0.1f);
}

GLuint createCombinedMapTexture(GLuint texValley, GLuint texMountain, GLuint texMap, GLuint texLake, GLuint texRoad) {
	GLuint fbo;
	GLuint tex;
	GLuint vao;

	programCombineMap = new glshaderprogram({"shaders/fsquad.vert", "shaders/combinedmaptexture.frag"});

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, tex_1k);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	GLenum attachment[] =  {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, attachment);

	programCombineMap->use();
	glBindTextureUnit(0, texValley);
	glBindTextureUnit(1, texMountain);
	glBindTextureUnit(2, texMap);
	glBindTextureUnit(3, texLake);
	glBindTextureUnit(4, texRoad);
	glViewport(0, 0, tex_1k);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDeleteVertexArrays(1, &vao);
	glDeleteFramebuffers(1, &fbo);

	return tex;
}

GLuint valleyHeightMap;
GLuint mountainHeightMap;
GLuint texLakeMap;
GLuint texRoadMap;

void dayscene::init() {
	dayevents = new eventmanager({
		{CROSSIN_T, { 0.0f, 1.0f }},
		{GODRAYIN_T, { 0.0f, 1.5f }},
		{CAMERA1MOVE_T, { 1.0f, 40.0f }},
		{DRONETURN_T, { 0.5f, 0.5f }},
		{DRONEMOVE_T, { 0.75f, 40.6f }},
		{ROVERMOVE_T, {53.0f, 8.0f}},
		{SUNRISEINIT_T, {41.0f, 5.0f}},
		{SUNRISEMID_T, {46.0f, 4.0f}},
		{SUNRISEEND_T, {50.0f, 4.0f}},
		{CAMERA2MOVE_T, { 54.0f, 30.0f }},
	});

	texDiffuseGrass = createTexture2D("resources/textures/grass.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseDirt = createTexture2D("resources/textures/dirt.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseMountain = createTexture2D("resources/textures/rocks2.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texLakeDuDvMap = createTexture2D("resources/textures/dudv.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
 
	ivec2 dim = ivec2(2048, 2048);
	valleyHeightMap = opensimplexnoise::createFBMTexture2D(dim, ivec2(0, 0), 900.0f, 1.0f, 3, 1234);
	mountainHeightMap = opensimplexnoise::createTurbulenceFBMTexture2D(dim, ivec2(0, 0), 1200.0f, 4, 0.11f, 111);
	texLakeMap = createTexture2D("resources/textures/lake.png", GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texRoadMap = createTexture2D("resources/textures/road.png", GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texTerrainMap = createTexture2D("resources/textures/map.png", GL_NEAREST, GL_NEAREST, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texTerrainHeight = createCombinedMapTexture(valleyHeightMap, mountainHeightMap, texTerrainMap, texLakeMap, texRoadMap);
	land = new terrain(texTerrainHeight, 256, true, 5.0f, 16.0f);

	modelLab = new glmodel("resources/models/spaceship/LabOut.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelRover = new glmodel("resources/models/rover/rover.glb", aiProcessPreset_TargetRealtime_Quality, true);
	// modelTreePine = new glmodel("resources/models/tree/pine.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelTreeRed = new glmodel("resources/models/tree/redtree.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	// modelTreePurple = new glmodel("resources/models/tree/purpletree.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelDrone = new glmodel("resources/models/drone/drone2.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelDrone->update(0.0f, 1);

	vector<vec3> droneVector = {
		vec3(64.1991f, -0.667602f, -79.8903f),
		vec3(78.5989f, -0.367602f, -81.6903f),
		vec3(81.3988f, -0.067602f, -74.1903f),
		vec3(83.0988f, 1.3324f, -66.4905f),
		vec3(72.299f, -2.1676f, -55.1906f),
		vec3(75.3989f, 0.232401f, -48.5908f),
		vec3(72.699f, 2.9324f, -33.691f),
		vec3(78.2989f, 6.1324f, -23.591f),
		vec3(92.6986f, 13.0324f, -10.7909f),
		vec3(86.6987f, 12.4324f, -1.19093f),
		vec3(74.5989f, 10.1324f, -6.39092f),
		vec3(58.3992f, 5.2324f, -18.091f),
		vec3(50.4993f, 5.2324f, -33.891f),
		vec3(50.3993f, 5.2324f, -47.3908f),
		vec3(53.3992f, 5.2324f, -63.6905f),
		vec3(55.979f, 7.71237f, -80.0917f),
		vec3(47.1792f, 7.71237f, -95.0915f),
		vec3(32.9794f, 7.71237f, -99.6914f),
		vec3(14.2794f, 8.21239f, -98.9911f),
		vec3(0.479343f, 3.51239f, -85.9913f),
		vec3(-12.4207f, -2.3876f, -83.0913f),
		vec3(-23.3207f, -3.88763f, -86.5912f),
		vec3(-34.0207f, -3.98763f, -94.6911f),
		vec3(-52.021f, -2.68763f, -98.891f),
		vec3(-67.3207f, 4.01237f, -99.4911f),
		vec3(-80.2207f, 12.1124f, -86.7912f),
		vec3(-87.9205f, 16.5124f, -44.2918f),
		vec3(-105.02f, 32.8124f, -29.092f)
	};
	splineDrone = new BsplineInterpolator(droneVector);
	vector<vec3> roverVector = {
		vec3(47.7993f, 1.0324f, -92.0901f),
		vec3(48.5994f, -1.7676f, -65.7905f),
		vec3(60.5991f, -2.3676f, -47.1907f),
		vec3(83.0988f, -1.8676f, -47.9908f)
	};
	splineRover = new BsplineInterpolator(roverVector);
	splineAdjuster = new SplineAdjuster(splineRover);
	splineAdjuster->setRenderPath(true);
	splineAdjuster->setRenderPoints(true);
	splineAdjuster->setScalingFactor(0.1f);

	godraysDrone = new godrays();
	godraysDrone->setDensity(1.1f);
	godraysDrone->setExposure(1.0f);
	godraysDrone->setSamples(100);
	godraysDrone->setWeight(0.01f);

	lightManager = new SceneLight();
	lightManager->addDirectionalLight(DirectionalLight(vec3(0.1f),10.0f,vec3(0.0,-1.0,0.0f)));

	lake1 = new lake(-6.0f);

	atmosphere = new Atmosphere();
#ifdef DEBUG
	// lakePlacer = new modelplacer(vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 10.0f);
	// vec3(-49.0f, -6.0f, -72.0f), vec3(0.0f, 0.0f, 0.0f), 38.0f -> Lake
	// vec3(53.1005f, -3.23743f, -56.8485f), vec3(0f, 0f, 0f), 0.00910002f -> Rover
	// vec3(-49.0f, -6.0f, -72.0f), vec3(0.0f), 38.0f
	// lakePlacer = new modelplacer(vec3(-30.4f, -0.2f, -58.0f), vec3(0.0f, 27.0f, 0.0f), 1.0f);
	// lakePlacer = new modelplacer(vec3(62.3101f, 0.6f, -74.39f), vec3(0.0f, -90.0f, 0.0f), 0.92f);
	lakePlacer = new modelplacer();
	// playerBkgnd = new audioplayer("resources/audio/TheLegendOfKaiOnlyScene2.wav");`
	playerBkgnd = new audioplayer("resources/audio/TheLegendOfKaiOnlyScene2.wav");
#endif
}

void dayscene::renderScene(bool cameraFlip) {
	mat4 currentViewMatrix = cameraFlip ? programglobal::currentCamera->matrixYFlippedOnPlane(lake1->getLakeHeight()) : programglobal::currentCamera->matrix();

	programTerrain->use();
	glUniformMatrix4fv(programTerrain->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programTerrain->getUniformLocation("vMat"), 1, GL_FALSE, currentViewMatrix);
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, -30.0f));
	glUniform1f(programTerrain->getUniformLocation("maxTess"), land->getMaxTess());
	glUniform1f(programTerrain->getUniformLocation("minTess"), land->getMinTess());
	glUniform3fv(programTerrain->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programTerrain->getUniformLocation("texHeight"), 0);
	// glUniform1i(programTerrain->getUniformLocation("texNormal"), 1);
	glUniform1i(programTerrain->getUniformLocation("texMap"), 2);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseGrass"), 3);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseDirt"), 4);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseMountain"), 5);
	glUniform1f(programTerrain->getUniformLocation("texScale"), 15.0f);
	glUniform1f(programTerrain->getUniformLocation("clipy"), lake1->getLakeHeight());
	glBindTextureUnit(0, land->getHeightMap());
	glBindTextureUnit(1, land->getNormalMap());
	glBindTextureUnit(2, texTerrainMap);
	glBindTextureUnit(3, texDiffuseGrass);
	glBindTextureUnit(4, texDiffuseDirt);
	glBindTextureUnit(5, texDiffuseMountain);
	lightManager->setLightUniform(programTerrain, false);
	land->render();
	for(int i = 0; i < 9; i++) {
		glBindTextureUnit(i, 0);
	}
	programStaticPBR->use();
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"), 1, GL_FALSE, currentViewMatrix);
	glUniform3fv(programStaticPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programStaticPBR->getUniformLocation("specularGloss"), GL_FALSE);
	glUniform1f(programStaticPBR->getUniformLocation("clipy"), lake1->getLakeHeight());
	lightManager->setLightUniform(programStaticPBR, false);
	
	// glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, lakePlacer->getModelMatrix());
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(-30.4f, 1.8f, -62.8999f) * rotate(27.0f, 0.0f, 1.0f, 0.0f) * scale(2.5f));
	modelTreeRed->draw(programStaticPBR);
	
	programDynamicPBR->use();
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"), 1, GL_FALSE, currentViewMatrix);
	glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"), GL_FALSE);
	glUniform1f(programDynamicPBR->getUniformLocation("clipy"), lake1->getLakeHeight());
	lightManager->setLightUniform(programDynamicPBR, false);
	modelDrone->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
	vec3 eye = splineDrone->interpolate((*dayevents)[DRONEMOVE_T]);
	vec3 front = splineDrone->interpolate((*dayevents)[DRONEMOVE_T] + 0.001f);
	mat4 mMatrix = translate(eye) * translate(0.0f, -1.9f, 0.0f) * rotate(-103.0f * (1.0f - (*dayevents)[DRONETURN_T]), 0.0f, 1.0f, 0.0f) * targetat(eye, front, vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"), 1, GL_FALSE, mMatrix * scale(15.0f));
	glUniform1i(programDynamicPBR->getUniformLocation("renderEmissiveToOcclusion"), 1);
	modelDrone->draw(programDynamicPBR);
	godraysDrone->setScreenSpaceCoords(programglobal::perspective * programglobal::currentCamera->matrix(), vec4(eye, 1.0f));
	glUniform1i(programDynamicPBR->getUniformLocation("renderEmissiveToOcclusion"), 0);

	if((*dayevents)[SUNRISEINIT_T] <= 0.0f) {
    	atmosphere->render(currentViewMatrix, mix(vec1(radians(-10.0f)), vec1(radians(0.0f)), (*dayevents)[CAMERA1MOVE_T])[0]);
	} else if((*dayevents)[SUNRISEMID_T] <= 0.0f) {
    	atmosphere->render(currentViewMatrix, mix(vec1(radians(0.0f)), vec1(radians(3.0f)), (*dayevents)[SUNRISEINIT_T])[0]);
	} else if((*dayevents)[SUNRISEEND_T] <= 0.0f) {
    	atmosphere->render(currentViewMatrix, mix(vec1(radians(3.0f)), vec1(radians(10.0f)), (*dayevents)[SUNRISEMID_T])[0]);
	} else {
    	atmosphere->render(currentViewMatrix, mix(vec1(radians(10.0f)), vec1(radians(35.0f)), (*dayevents)[SUNRISEEND_T])[0]);
	}
}

void dayscene::render() {
	camera1->setT((*dayevents)[CAMERA1MOVE_T]);
	camera2->setT((*dayevents)[CAMERA2MOVE_T]);

	godraysDrone->setDecay(mix(vec1(1.04f), vec1(0.95f), (*dayevents)[GODRAYIN_T])[0]);

	glEnable(GL_CLIP_DISTANCE0);
	lake1->setReflectionFBO();
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.3f, 0.6f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	this->renderScene(true);
	glDisable(GL_CLIP_DISTANCE0);

	glEnable(GL_CLIP_DISTANCE1);
	lake1->setRefractionFBO();
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.3f, 0.6f, 1.0f));
	glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
	this->renderScene();
	glDisable(GL_CLIP_DISTANCE1);

	resetFBO();
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.3f, 0.6f, 1.0f));

	this->renderScene();

	// programDynamicPBR->use();
	// glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	// glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	// glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
	// glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"), GL_FALSE);
	// lightManager->setLightUniform(programDynamicPBR, false);
	// programStaticPBR->use();
	// glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	// glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	// glUniform3fv(programStaticPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
	// glUniform1i(programStaticPBR->getUniformLocation("specularGloss"), GL_FALSE);
	// lightManager->setLightUniform(programStaticPBR, false);
	
	programDrawOnTerrain->use();
	glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniform3fv(programDrawOnTerrain->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programDrawOnTerrain->getUniformLocation("specularGloss"), GL_FALSE);
	glUniform1f(programDrawOnTerrain->getUniformLocation("heightMapScale"), 1.0f / 128.0f);
	glUniform1i(programDrawOnTerrain->getUniformLocation("heightMap"), 11);
	glBindTextureUnit(11, texTerrainHeight);
	lightManager->setLightUniform(programDrawOnTerrain, false);
	
	glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(63.3101f, 1.02f, -67.39f) * rotate(-90.0f, 0.0f, 1.0f, 0.0f) * scale(0.92f));
	// glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, lakePlacer->getModelMatrix());
	modelLab->draw(programDrawOnTerrain);

	vec3 pos = splineRover->interpolate((*dayevents)[ROVERMOVE_T]);
	vec3 front = splineRover->interpolate((*dayevents)[ROVERMOVE_T] + 0.01f);
	if((*dayevents)[ROVERMOVE_T] >= 0.0001f) {
		glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(pos) * targetat(pos, front, vec3(0.0f, 1.0f, 0.0f)) * scale(0.00910002f));
	} else {
		glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(75.0f, -1.8f, -68.0f) * rotate(8.0f, 0.0f, 0.0f, 1.0f) * scale(0.00910002f));
	}
	modelRover->draw(programDrawOnTerrain);
	
	// glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(61.0f, 6.7f, -53.4f) * scale(0.7f));
	// modelTreePine->draw(programDrawOnTerrain);
	
	// glUniform3fv(programDrawOnTerrain->getUniformLocation("position"), 1, xyzVector);
	// glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, scale(2.9f));
	// modelTreeRed->draw(programDrawOnTerrain);

  	// glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(48.0f, -5.0f, -34.0f) * scale(2.0f));
	// modelTreePurple->draw(programDrawOnTerrain);

	programLake->use();
	glUniformMatrix4fv(programLake->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programLake->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programLake->getUniformLocation("mMat"), 1, GL_FALSE, translate(-31.7f, lake1->getLakeHeight(), -62.0f) * scale(54.5f));
	glUniform1i(programLake->getUniformLocation("texRefraction"), 0);
	glUniform1i(programLake->getUniformLocation("texReflection"), 1);
	glUniform1i(programLake->getUniformLocation("texDuDv"), 2);
	glUniform1f(programLake->getUniformLocation("time"), lakeT);
	glUniform3fv(programLake->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1f(programLake->getUniformLocation("distortionScale"), 0.003f);
	glBindTextureUnit(0, lake1->getRefractionTexture());
	glBindTextureUnit(1, lake1->getReflectionTexture());
	glBindTextureUnit(2, texLakeDuDvMap);
	lake1->render();

	if(programglobal::debugMode == CAMERA) {
		// camRig1->render();
		camRig2->render();
	} else if(programglobal::debugMode == SPLINE) {
		splineAdjuster->render(RED_PINK_COLOR);
	} else if(programglobal::debugMode == LIGHT) {
		// render light src
		programLight->use();
		glUniformMatrix4fv(programLight->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
		glUniformMatrix4fv(programLight->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix()); 
		lightManager->renderSceneLights(programLight);
	}
	// glDisable(GL_DEPTH_TEST);
	// drawTexQuad->use();
	// glUniformMatrix4fv(drawTexQuad->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	// glUniformMatrix4fv(drawTexQuad->getUniformLocation("vMat"), 1, GL_FALSE, mat4::identity());
	// glUniformMatrix4fv(drawTexQuad->getUniformLocation("mMat"), 1, GL_FALSE, translate(-0.36f, 0.181f, -0.7f) * scale(0.1f));
	// glUniform1i(drawTexQuad->getUniformLocation("texture_diffuse"), 3);
	// glBindTextureUnit(0, lake1->getReflectionTexture());
	// glBindTextureUnit(1, lake1->getRefractionTexture());
	// glBindTextureUnit(2, lake1->getDepthTexture());
	// glBindTextureUnit(3, land->getNormalMap());
	// programglobal::shapeRenderer->renderQuad();
	// glEnable(GL_DEPTH_TEST);
}

void dayscene::update() {
	static const float DRONE_ANIM_SPEED = 0.8f;
	static const float LAKE_SPEED = 0.02f;
	
	dayevents->increment();
	lakeT += LAKE_SPEED * programglobal::deltaTime;
	if(dayevents->getT() > 0.01f) {
		// playerBkgnd->play();
	}
	if((*dayevents)[DRONETURN_T] >= 0.1f) {
		modelDrone->update(DRONE_ANIM_SPEED * programglobal::deltaTime, 1);
	}
	if((*dayevents)[CAMERA2MOVE_T] >= 1.0f) {
		crossfader::startSnapshot(texDaySceneFinal);
		atmosphere->render(programglobal::currentCamera->matrix(), radians(35.0f));
		crossfader::endSnapshot();
		playNextScene();
	}
}

void dayscene::reset() {
	dayevents->resetT();
}

void dayscene::uninit() {
	delete land;
	delete atmosphere;
}

void dayscene::keyboardfunc(int key) {
	if(programglobal::debugMode == MODEL) {
		lakePlacer->keyboardfunc(key);
	} else if(programglobal::debugMode == CAMERA) {
		// camRig1->keyboardfunc(key);
		camRig2->keyboardfunc(key);
	} else if(programglobal::debugMode == SPLINE) {
		splineAdjuster->keyboardfunc(key);
	} else if(programglobal::debugMode == LIGHT) {
		lightManager->SceneLightKeyBoardFunc(key);
	} else if(programglobal::debugMode == NONE) {
		atmosphere->keyboardfunc(key);
	}
	switch(key) {
	case XK_Up:
		(*dayevents) += 0.4f;
		break;
	case XK_Down:
		(*dayevents) -= 0.4f;
		break;
	case XK_F2:
		renderPath = !renderPath;
		camRig1->setRenderPathToFront(renderPath);
		camRig2->setRenderPathToFront(renderPath);
		break;
	case XK_Tab:
		if(programglobal::debugMode == CAMERA) {
			cout<<camRig1->getCamera()<<endl;
			cout<<camRig2->getCamera()<<endl;
		} else 
		if(programglobal::debugMode == SPLINE) {
			cout<<splineAdjuster->getSpline()<<endl;
		}
		if(programglobal::debugMode == MODEL) {
			cout<<lakePlacer<<endl;
		}
		cout<<"Current T = "<<dayevents->getT()<<endl;
		break;
	}
}

camera* dayscene::getCamera() {
	if((*dayevents)[CAMERA2MOVE_T] > 0.00001f) {
		return camera2;
	} else {
		return camera1;
	}
}

void dayscene::crossfade() {
	if((*dayevents)[DRONETURN_T] < 0.7f) {
		godraysDrone->renderRays();
	}
	if((*dayevents)[CROSSIN_T] < 1.0f) {
		crossfader::render(texLabSceneFinal, (*dayevents)[CROSSIN_T]);
	}
}