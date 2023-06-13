#define DEBUG
#include<scenes/night.h>
#include<iostream>
#include<vmath.h>
#include<scenecamera.h>
#include<scenecamerarig.h>
#include<modelplacer.h>
#include<X11/keysym.h>
#include<interpolators.h>
#include<splinerenderer.h>
#include<terrain.h>
#include<glLight.h>
#include<assimp/postprocess.h>
#include<global.h>
#include<opensimplexnoise.h>
#include<crossfade.h>
#include<eventmanager.h>
#include<splineadjuster.h>
#include<audio.h>
#include<godrays.h>
#include<sphere.h>
#include<gltextureloader.h>
#include<flock.h>
#include<glmodelloader.h>

using namespace std;
using namespace vmath;

static glshaderprogram* programTerrain;
static glshaderprogram* programStaticPBR;
static glshaderprogram* programStaticInstancedPBR;
static glshaderprogram* programDynamicPBR;
static glshaderprogram* programNightSky;
static glshaderprogram* programSkybox;
static glshaderprogram* programTex;
static terrain* land;
static terrain* land2;
static SceneLight* lightManager;
static sceneCamera* camera1;
static sphere* moon;
static modelplacer* quickModelPlacer;
static glmodel* modelTreeRed;
static glmodel* modelFlowerPurple;
static glmodel* modelPhoenix;

static godrays* godraysMoon;
#ifdef DEBUG
static sceneCameraRig* camRig1;
static bool renderPath = false;
#endif
extern GLuint texDaySceneFinal;
static GLuint texDiffuseGrass;
static GLuint texDiffuseDirt;
static GLuint uboTreePosition;
static GLuint uboFlowerPosition;
static GLuint uboTreeColor;
static GLuint vaoNightSky;
static GLuint vboNightSky;
static GLuint fboNightSky;
static GLuint texColorNightSky;
static GLuint texEmmissionNightSky;
static GLuint texMoon;
static GLuint skybox_vao;
static GLuint vbo;
static bool renderTrees = true;

enum tvalues {
	CROSSIN_T,
	CAMERAMOVE_T,
	FIREFLIES1BEGIN_T,
	FIREFLIES2BEGIN_T,
};
static eventmanager* nightevents;

static audioplayer* playerBkgnd;
static const int MAX_PARTICLES = 256;
static Flock *firefliesA = NULL;
static Flock *firefliesB = NULL;
static vec3 attractorPositionA =  vec3(-30.0f, 15.0f, -30.0f);
static vec3 attractorPositionB =  vec3(-30.0f, 15.0f, 30.0f);
static BsplineInterpolator *firefliesAPath1 = NULL;
static BsplineInterpolator *firefliesAPath2 = NULL;
static SplineRenderer *pathA1 = NULL;
static SplineRenderer *pathA2 = NULL;
static BsplineInterpolator *firefliesBPath1 = NULL;
static SplineRenderer *pathB1 = NULL;
static SplineAdjuster *pathAdjuster = NULL;

void nightscene::setupProgram() {
	try {
		programTerrain = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/renderjungle.frag"});
		programStaticPBR = new glshaderprogram({"shaders/pbrStatic.vert", "shaders/pbrMain.frag"});
		programStaticInstancedPBR = new glshaderprogram({"shaders/pbrStaticInstanced.vert", "shaders/trees.frag"});
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
		programSkybox = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/nightsky/rendercubemap.frag"});
		programTex = new glshaderprogram({"shaders/debug/basictex.vert", "shaders/debug/basictex.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void nightscene::setupCamera() {
	vector<vec3> positionVector = {
		//Look Down
		vec3(0.0f, 2.9f, -102.1f),
		vec3(0.0f, 2.6f, -102.1f),
		vec3(0.0f, 2.3f, -102.1f),
		vec3(0.0f, 2.0f, -102.1f),
		vec3(0.0f, 1.7f, -102.1f),
		//Look Around
		vec3(0.0f, 1.4f, -102.1f),
		vec3(0.0f, 1.4f, -102.1f),
		vec3(0.0f, 1.4f, -101.1f),
		vec3(0.0f, 1.4f, -100.1f),
		//Moon Look
		vec3(0.0f, 1.4f, -98.1f),
		vec3(0.0f, 1.4f, -96.1f),
		vec3(0.0f, 1.4f, -94.1f),
		vec3(0.0f, 1.4f, -92.1f),
		vec3(0.0f, 1.4f, -90.1f),
		vec3(0.0f, 1.4f, -86.1f),
		//Look Down
		vec3(0.0f, 1.4f, -80.1f),
		vec3(0.0f, 1.4f, -72.1f),
		vec3(0.0f, 1.4f, -62.1f),
		//Running
		vec3(0.0f, 1.4f, -50.1f),
		vec3(0.0f, 1.4f, -35.1f),
		vec3(0.0f, 1.4f, -20.1f),
		vec3(0.0f, 1.4f, -5.1f),
		vec3(0.0f, 1.4f, 15.1f),
		vec3(0.0f, 1.4f, 35.1f),
		vec3(0.0f, 1.4f, 55.1f),
		vec3(0.0f, 1.4f, 75.1f),
		vec3(0.0f, 1.4f, 95.1f),
		vec3(0.0f, 1.4f, 115.1f),
		vec3(0.0f, 1.4f, 135.1f),
		vec3(0.0f, 1.4f, 155.1f),
		vec3(0.0f, 1.4f, 175.1f),
		vec3(0.0f, 1.4f, 195.1f),
		vec3(0.0f, 1.4f, 215.1f),
		vec3(0.0f, 1.4f, 235.1f),
		vec3(0.0f, 1.4f, 255.1f),
		vec3(0.0f, 1.4f, 275.1f),
		vec3(0.0f, 1.4f, 295.1f),
		vec3(0.0f, 1.4f, 315.1f),
		vec3(0.0f, 1.4f, 335.1f),
		vec3(0.0f, 1.4f, 355.1f),
		vec3(0.0f, 1.4f, 375.1f),
		vec3(0.0f, 1.4f, 395.1f),
		vec3(0.0f, 1.4f, 415.1f),
		vec3(0.0f, 1.4f, 435.1f),
		vec3(0.0f, 1.4f, 455.1f),
		vec3(0.0f, 1.4f, 475.1f),
		vec3(0.0f, 1.4f, 495.1f),
		vec3(0.0f, 1.4f, 515.1f),
		vec3(0.0f, 1.4f, 535.1f),
		vec3(0.0f, 1.4f, 555.1f),
		vec3(0.0f, 1.4f, 575.1f),
	};
	
	vector<vec3> frontVector = {
		//Look Down
		vec3(13.8f, 23.81f, -102.2f),
		vec3(12.5f, 14.51f, -102.2f),
		vec3(8.6f, 6.61f, -102.2f),
		vec3(5.01f, 2.2f, -101.5f),
		vec3(3.1f, 1.5f, -98.8f),
		//Look Around
		vec3(-0.01f, 1.4f, -98.1f),
		vec3(-2.4f, 1.4f, -98.2f),
		vec3(-4.9f, 1.4f, -96.2f),
		vec3(-2.9f, 1.4f, -92.1f),
		//Moon Look
		vec3(-0.4f, 2.5f, -89.1f),
		vec3(0.0f, 4.3f, -85.3f),
		vec3(0.0f, 5.1f, -82.5f),
		vec3(0.0f, 5.2f, -80.4f),
		vec3(0.0f, 5.2f, -78.5f),
		vec3(0.0f, 4.6f, -75.9f),
		//Look Down
		vec3(0.0f, 3.2f, -74.1f),
		vec3(0.0f, 1.7f, -67.1f),
		vec3(0.0f, 1.4f, -58.1f),
		//Running
		vec3(0.0f, 1.4f, -45.1f),
		vec3(0.0f, 1.4f, -30.1f),
		vec3(0.0f, 1.4f, -15.1f),
		vec3(0.0f, 1.4f, -0.1f),
		vec3(0.0f, 1.4f, 20.1f),
		vec3(0.0f, 1.4f, 40.1f),
		vec3(0.0f, 1.4f, 60.1f),
		vec3(0.0f, 1.4f, 80.1f),
		vec3(0.0f, 1.4f, 100.1f),
		vec3(0.0f, 1.4f, 120.1f),
		vec3(0.0f, 1.4f, 140.1f),
		vec3(0.0f, 1.4f, 160.1f),
		vec3(0.0f, 1.4f, 180.1f),
		vec3(0.0f, 1.4f, 200.1f),
		vec3(0.0f, 1.4f, 220.1f),
		vec3(0.0f, 1.4f, 240.1f),
		vec3(0.0f, 1.4f, 260.1f),
		vec3(0.0f, 1.4f, 280.1f),
		vec3(0.0f, 1.4f, 300.1f),
		vec3(0.0f, 1.4f, 320.1f),
		vec3(0.0f, 1.4f, 340.1f), 
		vec3(0.0f, 1.4f, 360.1f),
		vec3(0.0f, 1.4f, 380.1f),
		vec3(0.0f, 1.4f, 400.1f),
		vec3(0.0f, 1.4f, 420.1f),
		vec3(0.0f, 1.4f, 440.1f),
		vec3(0.0f, 1.4f, 460.1f),
		vec3(0.0f, 1.4f, 480.1f),
		vec3(0.0f, 1.4f, 500.1f),
		vec3(0.0f, 1.4f, 520.1f),
		vec3(0.0f, 1.4f, 540.1f),
		vec3(0.0f, 1.4f, 560.1f),
		vec3(0.0f, 1.4f, 580.1f),
	};
	camera1 = new sceneCamera(positionVector, frontVector);

	camRig1 = new sceneCameraRig(camera1);
	camRig1->setRenderFront(true);
	camRig1->setRenderFrontPoints(true);
	camRig1->setRenderPath(true);
	camRig1->setRenderPathPoints(true);
	camRig1->setRenderPathToFront(renderPath);
	camRig1->setScalingFactor(0.1f);
}

void nightscene::init() {
	nightevents = new eventmanager({
		{CROSSIN_T, { 0.0f, 2.0f }},
		{CAMERAMOVE_T, { 2.0f, 80.0f }},
		{FIREFLIES1BEGIN_T, {8.0f, 30.0f}},
		{FIREFLIES2BEGIN_T, {20.0f, 30.0f}}
	});

	texDiffuseGrass = createTexture2D("resources/textures/grass.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseDirt = createTexture2D("resources/textures/dirt.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texMoon = createTexture2D("resources/textures/moon.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
	GLuint texJungle = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 0), 256.0f, 1.0f, 5, 235);
	GLuint texJungle2 = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 1024), 256.0f, 1.0f, 5, 235);

	playerBkgnd = new audioplayer("resources/audio/TheDragonWarrior.wav");

	godraysMoon = new godrays();
	godraysMoon->setDecay(0.98f);
	godraysMoon->setDensity(1.2f);
	godraysMoon->setExposure(1.0f);
	godraysMoon->setSamples(175);
	godraysMoon->setWeight(0.025f);
	
	// modelTreeRed = new glmodel("resources/models/tree/wtree.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelTreeRed = new glmodel("resources/models/tree/purpletree.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	
	float startz = -115.0f;
	float dz = 10.0f;
	float startx = -25.0f;
	float dx = 10.0f;
	vector<vec4> treePositionsArray;
	vector<vec4> flowerPositionsArray;
	
	int k = 0;
	for(int i = 0; i < 70; i++) {
		for(int j = 0; j < 6; j++) {
			treePositionsArray.push_back(vec4(startx + dx * j + programglobal::randgen->getRandomFloat(-4.0f, 4.0f), 0.0f, startz + dz * i + programglobal::randgen->getRandomFloat(-4.0f, 4.0f), 0.0f));
		}
	}

	for(int i = 0; i < 70; i++) {
		for(int j = 0; j < 6; j++) {
			flowerPositionsArray.push_back(vec4(startx + dx * j + programglobal::randgen->getRandomFloat(-5.0f, 5.0f), 0.0f, startz + dz * i + programglobal::randgen->getRandomFloat(-7.0f, 7.0f), 0.0f));
		}
	}

	// modelTreeRed = new glmodel("resources/models/tree/wtree.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelTreeRed = new glmodel("resources/models/tree/purpletree.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelFlowerPurple = new glmodel("resources/models/flowers/flower1.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelPhoenix = new glmodel("resources/models/phoenix/phoenix.glb", aiProcessPreset_TargetRealtime_Quality, true);

	glGenBuffers(1, &uboTreePosition);
	glBindBuffer(GL_UNIFORM_BUFFER, uboTreePosition);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4) * 420, treePositionsArray.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	vector<vec4> treeColors = {
		vec4(0.9f, 0.1f, 0.1f, 1.0f),
		vec4(0.1f, 0.1f, 0.9f, 1.0f),
		vec4(0.9f, 0.1f, 0.7f, 1.0f),
		vec4(0.9f, 0.7f, 0.1f, 1.0f),
		vec4(0.9f, 0.3f, 0.1f, 1.0f),
	};

	glGenBuffers(1, &uboTreeColor);
	glBindBuffer(GL_UNIFORM_BUFFER, uboTreeColor);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4) * treeColors.size(), treeColors.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &uboFlowerPosition);
	glBindBuffer(GL_UNIFORM_BUFFER, uboFlowerPosition);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4) * flowerPositionsArray.size(), flowerPositionsArray.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	vector<float> starArray;
	for(int i = 0; i < 300; i++) {
		starArray.push_back(programglobal::randgen->getRandomFloat(-1.0f, 1.0f));
		starArray.push_back(programglobal::randgen->getRandomFloat(-1.0f, 1.0f));
		starArray.push_back(programglobal::randgen->getRandomFloat(5.0f, 20.0f));
		starArray.push_back(programglobal::randgen->getRandomFloat(0.9f, 1.0f));
		starArray.push_back(programglobal::randgen->getRandomFloat(0.6f, 1.0f));
		starArray.push_back(programglobal::randgen->getRandomFloat(0.5f, 1.0f));
	}

	glGenVertexArrays(1, &vaoNightSky);
	glBindVertexArray(vaoNightSky);
	glGenBuffers(1, &vboNightSky);
	glBindBuffer(GL_ARRAY_BUFFER, vboNightSky);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * starArray.size(), starArray.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(sizeof(float) * 3));

	glGenTextures(1, &texColorNightSky);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texColorNightSky);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, 1024, 1024);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &texEmmissionNightSky);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texEmmissionNightSky);
	glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA32F, 1024, 1024);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPointSize(5.0f);
	programNightSky = new glshaderprogram({"shaders/nightsky/render.vert", "shaders/nightsky/render.geom", "shaders/nightsky/render.frag"});
	glGenFramebuffers(1, &fboNightSky);
	glBindFramebuffer(GL_FRAMEBUFFER, fboNightSky);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texColorNightSky, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, texEmmissionNightSky, 0);
	GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, drawBuffers);
	glBindFramebuffer(GL_FRAMEBUFFER, fboNightSky);
	glViewport(0, 0, 1024, 1024);
	glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	glClearBufferfv(GL_COLOR, 1, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	programNightSky->use();
	glBindVertexArray(vaoNightSky);
	glDrawArrays(GL_POINTS, 0, starArray.size() / 6);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	moon = new sphere(25, 50, 1.0f);
	land = new terrain(texJungle, 256, true, 5, 16);
	land2 = new terrain(texJungle2, 256, true, 5, 16);
	quickModelPlacer = new modelplacer();
	lightManager = new SceneLight(false);
	lightManager->addDirectionalLight(DirectionalLight(vec3(1.0f, 1.0f, 1.0f), 1.0f, vec3(0.0f, -0.5f, -1.0f)));

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

	firefliesA = new Flock(MAX_PARTICLES, attractorPositionA);
	firefliesB = new Flock(MAX_PARTICLES, attractorPositionB);

	vector<vec3> firefliesAPath1Points = vector<vec3>({
		vec3(-22.303f, 10.4127f, -80.3923f),
		vec3(-23.803f, 8.61268f, -84.9922f),
		vec3(-24.5208f, 3.2876f, -74.4906f),
		vec3(-16.6204f, 1.1876f, -70.8911f),
		vec3(-0.020374f, 1.3876f, -72.6911f),
		vec3(-23.3204f, 0.3876f, -52.3914f),
		vec3(20.2797f, 4.9876f, -49.9914f),
		vec3(-17.9204f, 0.6876f, -25.8917f),
		vec3(-4.82037f, 5.9876f, -9.1916f),
		vec3(17.5797f, 19.4876f, -8.9916f),
		vec3(26.7797f, 26.2877f, -29.9917f),
		vec3(-2.22034f, 33.8124f, -21.3931f)
	});
	firefliesAPath1 = new BsplineInterpolator(firefliesAPath1Points);
	pathA1 = new SplineRenderer(firefliesAPath1);
	vector<vec3> firefliesBPath1Points = vector<vec3>({
		vec3(25.3797f, 11.5876f, -7.19158f),
		vec3(-17.9204f, 0.9876f, 12.1084f),
		vec3(-4.72037f, 14.3876f, -28.2917f),
		vec3(-15.0204f, 20.1876f, 1.9084f),
		vec3(29.6797f, 27.3877f, -11.9916f),
		vec3(-3.82034f, 39.6123f, -36.8931f)
	});
	firefliesBPath1 = new BsplineInterpolator(firefliesBPath1Points);
	pathA2 = new SplineRenderer(firefliesBPath1);
	pathAdjuster = new SplineAdjuster(firefliesAPath1);
}

void nightscene::render() {
	camera1->setT((*nightevents)[CAMERAMOVE_T]);

	programSkybox->use();
	glUniformMatrix4fv(programSkybox->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
	glUniformMatrix4fv(programSkybox->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix());
	glUniform1i(programSkybox->getUniformLocation("skyboxColor"), 0);
	glUniform1i(programSkybox->getUniformLocation("skyboxEmmission"), 1);
	glUniform1f(programSkybox->getUniformLocation("emmissionPower"), (*nightevents)[CROSSIN_T]);
	glBindVertexArray(skybox_vao);
	glBindTextureUnit(0, texColorNightSky);
	glBindTextureUnit(1, texEmmissionNightSky);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	programTerrain->use();
	glUniformMatrix4fv(programTerrain->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programTerrain->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, scale(1.5f));
	glUniform1f(programTerrain->getUniformLocation("maxTess"), land->getMaxTess());
	glUniform1f(programTerrain->getUniformLocation("minTess"), land->getMinTess());
	glUniform3fv(programTerrain->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programTerrain->getUniformLocation("texHeight"), 0);
	// glUniform1i(programTerrain->getUniformLocation("texNormal"), 1);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseGrass"), 3);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseDirt"), 4);
	glUniform1f(programTerrain->getUniformLocation("texScale"), 15.0f);
	glBindTextureUnit(0, land->getHeightMap());
	glBindTextureUnit(1, land->getNormalMap());
	glBindTextureUnit(3, texDiffuseGrass);
	glBindTextureUnit(4, texDiffuseDirt);
	lightManager->setLightUniform(programTerrain, false);
	land->render();
	
	glUniformMatrix4fv(programTerrain->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programTerrain->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, 256.0f * 1.5f) * scale(1.5f));
	glUniform1f(programTerrain->getUniformLocation("maxTess"), land->getMaxTess());
	glUniform1f(programTerrain->getUniformLocation("minTess"), land->getMinTess());
	glUniform3fv(programTerrain->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programTerrain->getUniformLocation("texHeight"), 0);
	// glUniform1i(programTerrain->getUniformLocation("texNormal"), 1);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseGrass"), 2);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseDirt"), 3);
	glUniform1f(programTerrain->getUniformLocation("texScale"), 15.0f);
	glBindTextureUnit(0, land2->getHeightMap());
	glBindTextureUnit(1, land2->getNormalMap());
	glBindTextureUnit(2, texDiffuseGrass);
	glBindTextureUnit(3, texDiffuseDirt);
	lightManager->setLightUniform(programTerrain, false);
	land2->render();
	for(int i = 0; i < 9; i++) {
		glBindTextureUnit(i, 0);
	}

	firefliesA->renderAsSpheres(vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f), 0.05f);
	firefliesA->renderAttractorAsQuad(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.25f);
	if(renderTrees) {
		programStaticInstancedPBR->use();
		glUniformMatrix4fv(programStaticInstancedPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
		glUniformMatrix4fv(programStaticInstancedPBR->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
		glUniformMatrix4fv(programStaticInstancedPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 4.0f, 0.0f) * rotate(-90.0f,vec3(1.0f,0.0f,0.0f)) * scale(1.0f));
		lightManager->setLightUniform(programStaticInstancedPBR, false);
		glUniform3fv(programStaticInstancedPBR->getUniformLocation("emissionColor"),1,vec3(0.05f,0.05f,0.05f));
		glBindBufferBase(GL_UNIFORM_BUFFER, programStaticInstancedPBR->getUniformLocation("position_ubo"), uboTreePosition);
		glBindBufferBase(GL_UNIFORM_BUFFER, programStaticInstancedPBR->getUniformLocation("color_ubo"), uboTreeColor);
		float z = programglobal::currentCamera->position()[2];
		int count = 120;
		if(z >= -95.0f) {
			z += 95.0f;
			int n = 6 * (int)(z / 10.0f);
			count = std::min(420 - n, 120);
			glUniform1i(programStaticInstancedPBR->getUniformLocation("instanceOffset"), n);
		} else {
			glUniform1i(programStaticInstancedPBR->getUniformLocation("instanceOffset"), 0);
		}
		modelTreeRed->draw(programStaticInstancedPBR, count);

		count = 200;
		if(z >= -95.0f) {
			//z += 95.0f;
			int n = 8 * (int)(z / 10.0f);
			count = std::min(420 - n, 200);
			glUniform1i(programStaticInstancedPBR->getUniformLocation("instanceOffset"), n);
		} else {
			glUniform1i(programStaticInstancedPBR->getUniformLocation("instanceOffset"), 0);
		}
		glUniformMatrix4fv(programStaticInstancedPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.3f, 0.0f) * rotate(0.0f,vec3(1.0f,0.0f,0.0f)) * scale(1.0f));
		glUniform3fv(programStaticInstancedPBR->getUniformLocation("emissionColor"),1,vec3(0.0,0.0,0.0));
		glBindBufferBase(GL_UNIFORM_BUFFER, programStaticInstancedPBR->getUniformLocation("position_ubo"), uboFlowerPosition);
		glBindBufferBase(GL_UNIFORM_BUFFER, programStaticInstancedPBR->getUniformLocation("color_ubo"), uboTreeColor);
		modelFlowerPurple->draw(programStaticInstancedPBR, count);
	}
	// firefliesA->renderAsSpheres(vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f), 0.05f);
	// firefliesA->renderAttractorAsQuad(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.25f);

	// firefliesB->renderAsSpheres(vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f), 0.05f);
	// firefliesB->renderAttractorAsQuad(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.25f);

	//phoenix test
	programDynamicPBR->use();
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
    glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,5.0f,0.0f) * scale(10.0f,10.0f,10.0f));
    modelPhoenix->update(0.01f, 0);
    modelPhoenix->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
	glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"),1,programglobal::currentCamera->position());
	glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"),false);
	lightManager->setLightUniform(programDynamicPBR, false);
	modelPhoenix->draw(programDynamicPBR,1);

	programTex->use();
	glUniformMatrix4fv(programTex->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programTex->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programTex->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 320.0f, 520.0f) * scale(35.0f));
	glUniform1i(programTex->getUniformLocation("texture_diffuse"), 0);
	glUniform1i(programTex->getUniformLocation("texture_emmission"), 1);
	glUniform1i(programTex->getUniformLocation("texture_occlusion"), 2);
	glBindTextureUnit(0, texMoon);
	glBindTextureUnit(1, 0);
	glBindTextureUnit(2, texMoon);
	moon->render();
	godraysMoon->setScreenSpaceCoords(programglobal::perspective * programglobal::currentCamera->matrix() * translate(0.0f, 297.0f, 517.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f));

	if((*nightevents)[CROSSIN_T] < 1.0f) {
		crossfader::render(texDaySceneFinal, (*nightevents)[CROSSIN_T]);
	}

	if(programglobal::debugMode == CAMERA) {
		camRig1->render();
		pathAdjuster->render(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f));
	} else if(programglobal::debugMode == SPLINE) {
		camRig1->render();
		pathAdjuster->render(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f));
	}
}

void nightscene::update() {
	nightevents->increment();
	attractorPositionA = vec3(firefliesAPath1->interpolate((*nightevents)[FIREFLIES1BEGIN_T]));
	firefliesA->setAttractorPosition(attractorPositionA);
	firefliesA->update();
	attractorPositionB = vec3(firefliesBPath1->interpolate((*nightevents)[FIREFLIES2BEGIN_T]));
	firefliesB->update();
	firefliesB->setAttractorPosition(attractorPositionB);
	if((*nightevents)[CROSSIN_T] >= 0.01f) {
		if(programglobal::isAnimating) {
			playerBkgnd->play();
		} else {
			playerBkgnd->pause();
		}
	}
}

void nightscene::reset() {
	nightevents->resetT();
	playerBkgnd->restart();
}

void nightscene::uninit() {
	delete land;
	delete pathAdjuster;
	delete pathA1;
	delete pathA2;
	delete firefliesAPath1;
	delete firefliesAPath2;
	delete firefliesB;
	delete firefliesA;
}

void nightscene::keyboardfunc(int key) {
	if(programglobal::debugMode == MODEL) {
		quickModelPlacer->keyboardfunc(key);
	} else if(programglobal::debugMode == CAMERA) {
		camRig1->keyboardfunc(key);
	} else if(programglobal::debugMode == SPLINE) {
		pathAdjuster->keyboardfunc(key);
	} else if(programglobal::debugMode == NONE) {
	}
	switch(key) {
	case XK_Up:
		(*nightevents) += 0.4f;
		break;
	case XK_Down:
		(*nightevents) -= 0.4f;
		break;
	case XK_F2:
		renderPath = !renderPath;
		camRig1->setRenderPathToFront(renderPath);
		break;
	case XK_F9:
		delete pathAdjuster;
		pathAdjuster = new SplineAdjuster(firefliesAPath1);
		pathAdjuster->setScalingFactor(0.1f);
		pathAdjuster->setRenderPoints(true);
		break;
	case XK_F10:
		delete pathAdjuster;
		pathAdjuster = new SplineAdjuster(firefliesBPath1);
		pathAdjuster->setScalingFactor(0.1f);
		pathAdjuster->setRenderPoints(true);
		break;
	case XK_F12:
		renderTrees = !renderTrees;
		break;
	case XK_Tab:
		if(programglobal::debugMode == CAMERA) {
			cout<<camRig1->getCamera()<<endl;
		} else if(programglobal::debugMode == SPLINE) {
			cout << pathAdjuster->getSpline() << endl;
		} else if(programglobal::debugMode == MODEL) {
			cout<<quickModelPlacer<<endl;
		}
		break;
	}
}

camera* nightscene::getCamera() {
	return camera1;
}

void nightscene::crossfade() {
	godraysMoon->renderRays();
}