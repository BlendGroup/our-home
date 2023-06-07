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

using namespace std;
using namespace vmath;

static glshaderprogram* programTerrain;
static glshaderprogram* programLake;
static glshaderprogram* programStaticPBR;
static glshaderprogram* programDynamicPBR;
static glshaderprogram* programColor;
static glshaderprogram* programCombineMap;

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

static godrays* godraysDrone;

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
	
enum tvalues {
	CROSSIN_T,
	GODRAYIN_T,
	CAMERA1MOVE_T,
	CAMERA2MOVE_T,
	DRONETURN_T,
	DRONEMOVE_T,
};
static eventmanager* dayevents;

void dayscene::setupProgram() {
	try {
		programTerrain = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/render.frag"});
		programLake = new glshaderprogram({"shaders/lake/render.vert", "shaders/lake/render.frag"});
		programStaticPBR = new glshaderprogram({"shaders/pbrStatic.vert", "shaders/pbrMain.frag"});
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
		programColor = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
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
		vec3(64.3364f, -0.71273f, -77.4934f),
		vec3(61.9965f, -0.31273f, -82.6933f),
		vec3(52.0967f, 1.88727f, -89.4932f),
		vec3(36.5796f, 6.2124f, -98.2914f),
		vec3(26.7796f, 11.2124f, -105.591f),
		vec3(-0.720439f, 14.7124f, -116.891f),
		vec3(-36.3205f, 14.7124f, -130.091f),
		vec3(-71.8202f, 16.6124f, -130.991f),
		vec3(-82.7206f, 16.1124f, -122.591f),
		vec3(-86.9206f, 18.4124f, -109.391f)
	};
	vector<vec3> frontVector = {
		vec3(63.4991f, -0.667602f, -79.8903f),
		vec3(53.9795f, 1.3124f, -88.0908f),
		vec3(28.2794f, 8.21237f, -89.3914f),
		vec3(23.3794f, 7.81237f, -93.8913f),
		vec3(1.47934f, 5.21239f, -93.7911f),
		vec3(-20.0207f, 3.11237f, -102.891f),
		vec3(-29.7207f, 3.11237f, -107.491f),
		vec3(-52.021f, 1.51237f, -106.091f),
		vec3(-67.3207f, 4.01237f, -105.091f),
		vec3(-71.5208f, 10.3124f, -102.791f),
		vec3(-74.3207f, 15.9124f, -89.1912f)
	};
	camera1 = new sceneCamera(positionVector, frontVector);

	// camera 2 of sceneplay
	camRig2 = new sceneCameraRig(camera1);
	camRig2->setRenderFront(true);
	camRig2->setRenderFrontPoints(true);
	camRig2->setRenderPath(true);
	camRig2->setRenderPathPoints(true);
	camRig2->setRenderPathToFront(true);
	camRig2->setScalingFactor(0.1f);

	vector<vec3> positionVector2 = {
		// vec3(59.9365f, 59.4869f, 50.2058f),
		// vec3(78.9962f, 7.08727f, -86.9932f),
		// vec3(41.4969f, 12.9873f, -128.593f),
		// vec3(13.7796f, 12.8124f, -130.591f),
		// vec3(-18.6205f, 15.5124f, -135.791f),
		// vec3(-57.7201f, 12.6124f, -138.192f),
		// vec3(-87.2198f, 8.81238f, -104.491f),
		// vec3(-96.1198f, 4.61237f, -74.9917f),
		// vec3(-100.32f, 5.31237f, -30.9924f),
		// vec3(-88.5202f, 14.2124f, -8.99232f),
		// vec3(-72.7204f, 10.7124f, -25.8924f),
		// vec3(-50.3208f, -0.587627f, -61.392f),
		// vec3(-25.6214f, 2.1124f, -61.0918f),
		vec3(59.5365f, 59.4869f, 50.6058f),
		vec3(78.9962f, 7.08727f, -86.9932f),
		vec3(32.697f, 12.3873f, -118.093f),
		vec3(6.77958f, 14.4124f, -118.591f),
		vec3(-13.3205f, 10.2124f, -112.691f),
		vec3(-45.1203f, -0.687612f, -95.2919f),
		vec3(-40.7204f, 1.31239f, -53.5925f),
		vec3(-2.32041f, 1.81239f, -60.3924f),
		vec3(40.6794f, 1.01238f, -60.9917f),
	};
	vector<vec3> frontVector2 = {
		// vec3(61.8991f, 1.6324f, -67.0905f),
		// vec3(53.9795f, 1.3124f, -88.0908f),
		// vec3(30.6794f, 9.61237f, -108.091f),
		// vec3(9.27935f, 11.4124f, -105.291f),
		// vec3(-16.5207f, 12.5124f, -108.491f),
		// vec3(-48.1205f, 11.3124f, -105.491f),
		// vec3(-63.6203f, 6.11237f, -90.8911f),
		// vec3(-72.0207f, 5.11237f, -72.2915f),
		// vec3(-72.8207f, 3.51239f, -51.2917f),
		// vec3(-50.6211f, 1.51239f, -43.592f),
		// vec3(-39.4213f, 1.51239f, -48.3919f),
		// vec3(-31.6214f, 2.51239f, -61.2917f),
		// vec3(0.778743f, 1.01237f, -59.2916f),
		vec3(61.8991f, 1.6324f, -67.0905f),
		vec3(53.9795f, 1.3124f, -88.0908f),
		vec3(16.8793f, 13.6124f, -107.991f),
		vec3(-0.220681f, 13.3124f, -100.591f),
		vec3(-30.7207f, 3.71239f, -93.7912f),
		vec3(-37.6207f, -0.887612f, -64.0917f),
		vec3(23.7794f, 1.31239f, -73.1915f),
		vec3(42.5792f, 1.71237f, -63.8915f),
	};
	camera2 = new sceneCamera(positionVector2, frontVector2);

	camRig2 = new sceneCameraRig(camera2);
	camRig2->setRenderFront(true);
	camRig2->setRenderFrontPoints(true);
	camRig2->setRenderPath(true);
	camRig2->setRenderPathPoints(true);
	camRig2->setRenderPathToFront(true);
	camRig2->setScalingFactor(0.1f);
}

GLuint createCombinedMapTexture(GLuint texValley, GLuint texMountain, GLuint texMap, GLuint texLake) {
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
	glViewport(0, 0, tex_1k);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDeleteVertexArrays(1, &vao);
	glDeleteFramebuffers(1, &fbo);

	return tex;
}

void dayscene::init() {
	dayevents = new eventmanager({
		{CROSSIN_T, { 0.0f, 1.0f }},
		{GODRAYIN_T, { 0.0f, 2.0f }},
		{CAMERA1MOVE_T, { 1.0f, 23.0f }},
		{CAMERA2MOVE_T, { 1.0f, 46.0f }},
		{DRONETURN_T, { 0.5f, 1.5f }},
		{DRONEMOVE_T, { 1.0f, 25.6f }}
	});

	texDiffuseGrass = createTexture2D("resources/textures/grass.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseDirt = createTexture2D("resources/textures/dirt.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseMountain = createTexture2D("resources/textures/rocks2.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texLakeDuDvMap = createTexture2D("resources/textures/dudv.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
 
	ivec2 dim = ivec2(2048, 2048);
	GLuint valleyHeightMap = opensimplexnoise::createFBMTexture2D(dim, ivec2(0, 0), 900.0f, 3, 1234);
	GLuint mountainHeightMap = opensimplexnoise::createTurbulenceFBMTexture2D(dim, ivec2(0, 0), 1200.0f, 4, 0.11f, 543);
	GLuint texLakeMap = createTexture2D("resources/textures/lake.png", GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texTerrainMap = createTexture2D("resources/textures/map.png", GL_NEAREST, GL_NEAREST, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texTerrainHeight = createCombinedMapTexture(valleyHeightMap, mountainHeightMap, texTerrainMap, texLakeMap);
	land = new terrain(texTerrainHeight, 256, true, 5.0f, 16.0f);

	modelLab = new glmodel("resources/models/spaceship/LabOut.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelRover = new glmodel("resources/models/rover/rover.glb", aiProcessPreset_TargetRealtime_Quality, true);
	// modelTreePine = new glmodel("resources/models/tree/pine.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelTreeRed = new glmodel("resources/models/tree/redtree.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	// modelTreePurple = new glmodel("resources/models/tree/purpletree.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelDrone = new glmodel("resources/models/drone/drone2.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelDrone->update(0.0f, 1);

	vector<vec3> droneVector = {
		vec3(63.4991f, -0.667602f, -79.8903f),
		vec3(54.4795f, 0.7124f, -87.8908f),
		vec3(33.2794f, 7.71237f, -89.1915f),
		vec3(21.2794f, 10.1124f, -88.9912f),
		vec3(5.17934f, 10.5124f, -85.8913f),
		vec3(-12.4207f, 7.91239f, -83.0913f),
		vec3(-23.3207f, -3.88763f, -86.5912f),
		vec3(-34.0207f, -3.98763f, -94.6911f),
		vec3(-52.021f, -4.88763f, -98.891f),
		vec3(-67.3207f, 4.01237f, -99.4911f),
		vec3(-80.2207f, 12.1124f, -86.7912f),
		vec3(-87.9205f, 16.5124f, -44.2918f),
		vec3(-105.02f, 32.8124f, -29.092f)
	};
	splineDrone = new BsplineInterpolator(droneVector);
	splineAdjuster = new SplineAdjuster(splineDrone);
	splineAdjuster->setRenderPath(true);
	splineAdjuster->setRenderPoints(true);
	splineAdjuster->setScalingFactor(0.1f);

	godraysDrone = new godrays();
	godraysDrone->setDensity(1.0f);
	godraysDrone->setExposure(1.0f);
	godraysDrone->setSamples(100);
	godraysDrone->setWeight(0.01f);

	lightManager = new SceneLight();
	lightManager->addDirectionalLight(DirectionalLight(vec3(0.1f),10.0f,vec3(0.0,0.0,-1.0f)));
	
	lake1 = new lake(-6.0f);

#ifdef DEBUG
	// lakePlacer = new modelplacer(vec3(0.0f, 10.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 10.0f);
	// vec3(-49.0f, -6.0f, -72.0f), vec3(0.0f, 0.0f, 0.0f), 38.0f -> Lake
	// vec3(53.1005f, -3.23743f, -56.8485f), vec3(0f, 0f, 0f), 0.00910002f -> Rover
	// vec3(-49.0f, -6.0f, -72.0f), vec3(0.0f), 38.0f
	lakePlacer = new modelplacer();
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
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(-30.4f, -0.2f, -58.0f) * rotate(27.0f, 0.0f, 1.0f, 0.0f));
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
	mat4 mMatrix = translate(eye) * translate(0.1f, -1.9f, 0.1f) * rotate(170.0f * (1.0f - (*dayevents)[DRONETURN_T]), 0.0f, 1.0f, 0.0f) * targetat(eye, front, vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"), 1, GL_FALSE, mMatrix * scale(15.0f));
	glUniform1i(programDynamicPBR->getUniformLocation("renderEmissiveToOcclusion"), 1);
	modelDrone->draw(programDynamicPBR);
	godraysDrone->setScreenSpaceCoords(programglobal::perspective * programglobal::currentCamera->matrix(), vec4(eye, 1.0f));
	glUniform1i(programDynamicPBR->getUniformLocation("renderEmissiveToOcclusion"), 0);
}

vec3 xyzVector = vec3(0.0f, 0.0f, 0.0f);

static float goddecay = 1.04f;

void dayscene::render() {
	// camera1->setT((*dayevents)[CAMERA1MOVE_T]);
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
	
	// glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(61.8599f, -6.7f, -69.4705f) * scale(0.87f));
	glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(61.8599f, 0.0f, -69.4705f) * scale(0.87f));
	modelLab->draw(programDrawOnTerrain);

	glUniformMatrix4fv(programDrawOnTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(53.1005f, -3.23743f, -56.8485f) * scale(0.00910002f));
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
	glUniform1f(programLake->getUniformLocation("distortionScale"), 0.006f);
	glBindTextureUnit(0, lake1->getRefractionTexture());
	glBindTextureUnit(1, lake1->getReflectionTexture());
	glBindTextureUnit(2, texLakeDuDvMap);
	lake1->render();

	if(programglobal::debugMode == CAMERA) {
		// camRig1->render();
		camRig2->render();
	} else if(programglobal::debugMode == SPLINE) {
		splineAdjuster->render(RED_PINK_COLOR);
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
	static const float LAKE_SPEED = 0.03f;
	
	dayevents->increment();
	lakeT += LAKE_SPEED * programglobal::deltaTime;
	if(dayevents->getT() > 0.01f) {
		playerBkgnd->play();
	}
	if((*dayevents)[DRONETURN_T] >= 0.0f) {
		modelDrone->update(DRONE_ANIM_SPEED * programglobal::deltaTime, 1);
	}
	goddecay -= 0.0003f;
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
		// camRig1->keyboardfunc(key);
		camRig2->keyboardfunc(key);
	} else if(programglobal::debugMode == SPLINE) {
		splineAdjuster->keyboardfunc(key);
	} else if(programglobal::debugMode == NONE) {
		switch(key) {
		case XK_o:
			xyzVector[1] += 0.1f;
			break;
		case XK_u:
			xyzVector[1] -= 0.1f;
			break;
		case XK_i:
			xyzVector[2] += 0.1f;
			break;
		case XK_k:
			xyzVector[2] -= 0.1f;
			break;
		case XK_l:
			xyzVector[0] += 0.1f;
			break;
		case XK_j:
			xyzVector[0] -= 0.1f;
			break;
		}
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
		// camRig1->setRenderPathToFront(renderPath);
		camRig2->setRenderPathToFront(renderPath);
		break;
	case XK_Tab:
		if(programglobal::debugMode == CAMERA) {
			// cout<<camRig1->getCamera()<<endl;
			cout<<camRig2->getCamera()<<endl;
		} else 
		if(programglobal::debugMode == SPLINE) {
			cout<<splineAdjuster->getSpline()<<endl;
		}
		if(programglobal::debugMode == MODEL) {
			cout<<lakePlacer<<endl;
		}
		break;
	}
}

camera* dayscene::getCamera() {
	// return camera1;
	return camera2;
}

void dayscene::crossfade() {
	if((*dayevents)[DRONETURN_T] < 0.7f) {
		godraysDrone->renderRays();
	}
	if((*dayevents)[CROSSIN_T] < 1.0f) {
		crossfader::render(texLabSceneFinal, (*dayevents)[CROSSIN_T]);
	}
}