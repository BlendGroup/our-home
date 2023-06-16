#include "CubeMapRenderTarget.h"
#include<glshaderloader.h>
#include<shapes.h>
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
#include<glmodelloader.h>
#include<errorlog.h>
#include<ocean.h>
#include<fireflies.h>

//Set to 0 to disable opencl or call me on my cell phone
#define OPENCL 0

using namespace std;
using namespace vmath;

static glshaderprogram* programTerrain;
static glshaderprogram* programStaticPBR;
static glshaderprogram* programStaticInstancedPBR;
static glshaderprogram* programDynamicPBR;
static glshaderprogram* programNightSky;
static glshaderprogram* programSkybox;
static glshaderprogram* programTex;
static glshaderprogram* programOcean;
static glshaderprogram* programFire;
static terrain* land;
static terrain* land2;
static terrain* island;
static SceneLight* lightManager;
static sceneCamera* camera1;
static sceneCamera* camera2;
static sphere* moon;
static modelplacer* quickModelPlacer;
static glmodel* modelTie;
static glmodel* modelTreeRed;
static glmodel* modelDrone;
static glmodel* modelAstro;
static glmodel* modelRover;
static glmodel* modelFlowerPurple;
static glmodel* modelPhoenix;
static glmodel* modelFox;
static glmodel* modelRocket;

static godrays* godraysMoon;
#ifdef DEBUG
static sceneCameraRig* camRig1;
static bool renderPath = false;
#endif
static vec3 sunDirection = vec3(-1.0f, 1.0f, 1.0f);
static vec3 oceanColor = vec3(0.004f, 0.016f, 0.047f);
static vec3 skyColor = vec3(0.0f, 0.028f, 0.06f);
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
static GLuint texOcean;
static GLuint skybox_vao;
static GLuint vbo;
static GLuint texBlack;
static bool renderTrees = true;
static float fireT = 0.0f;

enum tvalues {
	CROSSIN_T,
	CAMERAMOVE1_T,
	CAMERAMOVE2_T,
	FIREFLIES1BEGIN_T,
	FIREFLIES2BEGIN_T,
	FOXWALK_T,
	PHOENIXFLY_T,
	ROCKET_T,
	FADEOUT_T
};
static eventmanager* nightevents;

static audioplayer* playerBkgnd;
static const int MAX_PARTICLES = 128;
static Fireflies *firefliesA = NULL;
static Fireflies *firefliesB = NULL;
static SplineAdjuster *pathAdjuster = NULL;
static ocean *obocean;
static BsplineInterpolator* phoenixPath;
static SplineRenderer *pathPhoenix;

static CubeMapRenderTarget* iblNight;
static SceneLight* iblLight;
static bool iblSetup = true;
void nightscene::setupProgram() {
	try {
		programTerrain = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/renderjungle.frag"});
		programStaticPBR = new glshaderprogram({"shaders/pbrStatic.vert", "shaders/pbrMain.frag"});
		programStaticInstancedPBR = new glshaderprogram({"shaders/pbrStaticInstanced.vert", "shaders/trees.frag"});
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
		programSkybox = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/nightsky/rendercubemap.frag"});
		programTex = new glshaderprogram({"shaders/debug/basictex.vert", "shaders/debug/basictex.frag"});
		programOcean = new glshaderprogram({"shaders/ocean/ocean.vert", "shaders/ocean/ocean.frag"});
    	programFire = new glshaderprogram({"shaders/fire/render.vert", "shaders/fire/render.frag"});
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
		//Ocean
		vec3(0.0f, 1.4f, 600.1f),
		vec3(0.0f, 1.4f, 650.1f),
		vec3(0.0f, 1.4f, 700.1f),
		vec3(0.0f, 1.4f, 750.1f),
		vec3(0.0f, 1.4f, 800.1f),
		vec3(0.0f, 1.4f, 850.1f),
		vec3(0.0f, 1.4f, 900.1f),
		vec3(0.0f, 1.4f, 950.1f),
		vec3(0.0f, 1.4f, 1000.1f),
		vec3(0.0f, 1.0f, 1050.1f),
		vec3(0.0f, 0.6f, 1100.1f),
		vec3(0.0f, 0.2f, 1153.1f)
	};
	
	vector<vec3> frontVector = {
		vec3(13.8f, 23.81f, -102.2f),
		vec3(12.5f, 14.51f, -102.2f),
		vec3(8.6f, 6.61f, -102.2f),
		vec3(5.01f, 2.2f, -101.5f),
		vec3(3.1f, 1.5f, -98.8f),
		vec3(-0.01f, 1.4f, -98.1f),
		vec3(-2.4f, 1.4f, -98.2f),
		vec3(-4.9f, 1.4f, -96.2f),
		vec3(-2.9f, 1.4f, -92.1f),
		vec3(-0.4f, 2.5f, -89.1f),
		vec3(0.0f, 4.3f, -85.3f),
		vec3(0.0f, 5.1f, -82.5f),
		vec3(0.0f, 5.2f, -80.4f),
		vec3(0.0f, 5.2f, -78.5f),
		vec3(0.0f, 4.6f, -75.9f),
		vec3(0.0f, 3.2f, -74.1f),
		vec3(0.4f, 1.7f, -67.1f),
		vec3(1.3f, 1.4f, -58.1f),
		vec3(-1.6f, 1.4f, -45.1f),
		vec3(1.7f, 1.4f, -30.1f),
		vec3(0.0f, 1.4f, -15.1f),
		vec3(0.0f, 1.4f, -0.1f),
		vec3(0.0f, 1.4f, 20.1f),
		vec3(0.0f, 1.4f, 40.1f),
		vec3(2.6f, 2.1f, 60.1f),
		vec3(0.0f, 1.4f, 80.1f),
		vec3(0.0f, 1.4f, 100.1f),
		vec3(0.0f, 1.4f, 120.1f),
		vec3(1.5f, 1.4f, 140.1f),
		vec3(0.0f, 1.4f, 160.1f),
		vec3(0.0f, 1.4f, 180.1f),
		vec3(0.0f, 1.4f, 200.1f),
		vec3(0.0f, 1.4f, 220.1f),
		vec3(0.0f, 1.4f, 240.1f),
		vec3(0.0f, 1.4f, 260.1f),
		vec3(0.0f, 1.4f, 280.1f),
		vec3(-0.8f, 1.4f, 300.1f),
		vec3(0.0f, 1.4f, 320.1f),
		vec3(0.0f, 1.4f, 340.1f),
		vec3(0.0f, 2.2f, 360.1f),
		vec3(0.0f, 3.4f, 380.1f),
		vec3(0.0f, 3.4f, 400.1f),
		vec3(0.0f, 3.4f, 420.1f),
		vec3(0.0f, 3.4f, 440.1f),
		vec3(0.0f, 3.4f, 460.1f),
		vec3(0.0f, 3.4f, 480.1f),
		vec3(0.0f, 3.4f, 500.1f),
		vec3(0.0f, 3.4f, 520.1f),
		vec3(0.0f, 3.4f, 540.1f),
		vec3(0.0f, 2.2f, 560.1f),
		vec3(1.1f, 1.4f, 580.2f),
		vec3(1.1f, 1.4f, 605.1f),
		vec3(-0.7f, 2.3f, 655.1f),
		vec3(-0.7f, 3.4f, 705.1f),
		vec3(0.0f, 1.4f, 755.1f),
		vec3(0.0f, 1.4f, 805.1f),
		vec3(0.0f, 1.4f, 855.1f),
		vec3(0.0f, 1.4f, 905.1f),
		vec3(0.0f, 1.4f, 955.1f),
		vec3(0.0f, 1.4f, 1005.1f),
		vec3(0.0f, 1.0f, 1055.1f),
		vec3(0.0f, 0.6f, 1105.1f),
		vec3(0.0f, 0.2f, 1155.1f)
	};
	camera1 = new sceneCamera(positionVector, frontVector);

	vector<vec3> positionVector2 = {
		vec3(0.0f, 0.2f, 1153.1f),
		vec3(-2.5f, -0.4f, 1157.3f),
		vec3(-2.9f, -1.0f, 1165.5f),
		vec3(0.0f, -1.6f, 1171.1f)
	};
	vector<vec3> frontVector2 = {
		vec3(0.0f, 0.2f, 1155.1f),
		vec3(0.0f, 1.7f, 1155.1f),
		vec3(0.0f, 3.2f, 1155.1f),
		vec3(0.0f, 4.7f, 1155.1f),
	};
	camera2 = new sceneCamera(positionVector2, frontVector2);
	camRig1 = new sceneCameraRig(camera1);
	camRig1->setRenderFront(true);
	camRig1->setRenderFrontPoints(true);
	camRig1->setRenderPath(true);
	camRig1->setRenderPathPoints(true);
	camRig1->setRenderPathToFront(renderPath);
	camRig1->setScalingFactor(0.1f);
}

GLuint createCombinedMapTextureNight(GLuint texLand, GLuint texMap) {
	GLuint fbo;
	GLuint tex;
	GLuint vao;

	glshaderprogram* programCombineMap = new glshaderprogram({"shaders/fsquad.vert", "shaders/combinedmaptexturenight.frag"});

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
	glBindTextureUnit(0, texLand);
	glBindTextureUnit(1, texMap);
	glViewport(0, 0, tex_1k);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDeleteVertexArrays(1, &vao);
	glDeleteFramebuffers(1, &fbo);

	return tex;
}

void nightscene::init() {
	nightevents = new eventmanager({
		{CROSSIN_T, { 0.0f, 2.0f }},
		{CAMERAMOVE1_T, { 2.0f, 110.0f }},
		{CAMERAMOVE2_T, { 112.65f, 20.0f }},
		{FOXWALK_T, {11.1f, 6.0f}},
		{FIREFLIES1BEGIN_T, {25.0f, 55.0f}},//End at 80
		{FIREFLIES2BEGIN_T, {57.75f, 18.7f}}, //End at 80f
		{PHOENIXFLY_T, {69.0f, 30.0f}},
		{ROCKET_T, {127.0f, 38.0f}},
		{FADEOUT_T, {160.0f, 4.0f}},
	});

	texDiffuseGrass = createTexture2D("resources/textures/grass.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseDirt = createTexture2D("resources/textures/dirt.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texMoon = createTexture2D("resources/textures/moon.jpg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
	texOcean = createTexture2D("resources/textures/ocean.jpg", GL_LINEAR, GL_LINEAR);
	GLuint texMap = createTexture2D("resources/textures/islandmap.png", GL_LINEAR, GL_LINEAR);
	GLuint texJungle;
	GLuint texJungle2;
	GLuint texIsland;
#if OPENCL
	texJungle = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 0), 256.0f, 0.25f, 5, 235);
	texJungle2 = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 1024), 256.0f, 0.25f, 5, 235);
	texIsland = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 1024), 256.0f, 1.0f, 5, 235);
#else
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, vec1(0.0f));
	texJungle = tex;
	texJungle2 = tex;
	texIsland = tex;
#endif
	land = new terrain(texJungle, 64, true, 5, 8);
	land2 = new terrain(texJungle2, 64, true, 5, 8);
	GLuint texNewIsland = createCombinedMapTextureNight(texIsland, texMap);
	island = new terrain(texNewIsland, 256, true, 5, 16);
	
	GLubyte black[] = {0, 0, 0};
	glGenTextures(1, &texBlack);
	glBindTexture(GL_TEXTURE_2D, texBlack);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, black);
	glBindTexture(GL_TEXTURE_2D, 0);

	playerBkgnd = new audioplayer("resources/audio/TheDragonWarrior.wav");

	godraysMoon = new godrays();
	godraysMoon->setDecay(0.98f);
	godraysMoon->setDensity(1.2f);
	godraysMoon->setExposure(1.0f);
	godraysMoon->setSamples(175);
	godraysMoon->setWeight(0.025f);
	
	// modelTreeRed = new glmodel("resources/models/tree/wtree.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelTreeRed = new glmodel("resources/models/tree/purpletree.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelRover = new glmodel("resources/models/rover/rover.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelDrone = new glmodel("resources/models/drone/drone2.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelAstro = new glmodel("resources/models/astronaut/MCAnim.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelTie = new glmodel("resources/models/rover/tie.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelPhoenix = new glmodel("resources/models/phoenix/phoenix.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelFox = new glmodel("resources/models/phoenix/fox.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelRocket = new glmodel("resources/models/rover/spacex.glb", aiProcessPreset_TargetRealtime_Quality, true);

	obocean = new ocean(vec2(3.0f, 3.0f), 1.5f, 100);

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

	// modelFlowerPurple = new glmodel("resources/models/flowers/flower1.glb", aiProcessPreset_TargetRealtime_Quality, true);
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
	glClearBufferfv(GL_COLOR, 0, vec4(0.015f, 0.015f, 0.05f, 1.0f));
	glClearBufferfv(GL_COLOR, 1, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	programNightSky->use();
	glBindVertexArray(vaoNightSky);
	glDrawArrays(GL_POINTS, 0, starArray.size() / 6);
	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	moon = new sphere(25, 50, 1.0f);
	// quickModelPlacer = new modelplacer(vec3(-2.0f, -3.5f, 1161.0f), vec3(0.0f, 180.0f, 0.0f), 3.0f);
	// quickModelPlacer = new modelplacer(vec3(-9.8f, 0.03f, -95.1098f), vec3(0.0f, -77.0f, 0.0f), 0.05f);
	quickModelPlacer = new modelplacer();
	lightManager = new SceneLight(false);
	lightManager->addDirectionalLights({
		DirectionalLight(vec3(0.3f, 0.3f, 0.3f), 3.0f, vec3(0.0f, -1.0f, -1.0f)),
		DirectionalLight(vec3(0.3f, 0.3f, 0.3f), 3.0f, vec3(0.0f, -1.0f, 1.0f)),
	});

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

	vector<vec3> firefliesAPathPoints = vector<vec3>({
		// vec3(-23.8f, 8.6f, -85.0f),
		// vec3(-16.6f, 1.1f, -70.9f),
		// vec3(-1.8f, 0.7f, -72.7f),
		// vec3(0.0f, 1.7f, -67.1f),
		// vec3(3.3f, 3.2f, -57.1f),
		// vec3(-3.3f, 2.4f, -45.1f),
		// vec3(3.2f, 1.4f, -30.1f),
		// vec3(-1.1f, 3.0f, -3.9f),
		// vec3(0.0f, 1.4f, -0.1f),
		// vec3(0.0f, 1.4f, 20.1f),
		// vec3(5.3f, 3.4f, 43.4f),
		// vec3(5.0f, 4.1f, 62.6f),
		// vec3(0.0f, 1.8f, 80.1f),
		// vec3(0.0f, 1.4f, 100.1f),
		// vec3(-2.2f, 2.5f, 128.8f),
		// vec3(2.8f, 1.1f, 140.1f),
		// vec3(1.6f, 1.4f, 160.1f),
		// vec3(-0.3f, 1.4f, 180.1f),
		// vec3(1.8f, 1.4f, 200.1f),
		// vec3(-2.9f, 1.6f, 222.2f),
		// vec3(1.5f, 1.4f, 247.5f),
		// vec3(-0.7f, 1.4f, 268.0f),
		// vec3(0.0f, 1.4f, 280.1f),
		// vec3(-1.4f, 1.4f, 306.0f),
		// vec3(0.6f, 1.4f, 326.0f),
		// vec3(-16.0f, 4.9f, 356.4f),
		// vec3(12.7f, 4.9f, 360.5f),
		// vec3(4.1f, 10.1f, 340.797f),
		// vec3(-11.3f, 13.0f, 352.997f),
		// vec3(0.0f, 14.0f, 360.696f),
		// vec3(9.8f, 14.3f, 356.295f),
		// vec3(4.0f, 20.3f, 340.493f)
		vec3(-23.8f, 8.6f, -85.0f),
		vec3(-16.6f, 1.1f, -70.9f),
		vec3(-1.8f, 0.7f, -72.7f),
		vec3(0.0f, 1.7f, -67.1f),
		vec3(3.3f, 3.2f, -57.1f),
		vec3(-3.3f, 2.4f, -45.1f),
		vec3(3.2f, 1.4f, -30.1f),
		vec3(-1.1f, 3.0f, -3.9f),
		vec3(0.0f, 1.4f, -0.1f),
		vec3(0.0f, 1.4f, 20.1f),
		vec3(5.3f, 3.4f, 43.4f),
		vec3(5.0f, 4.1f, 62.6f),
		vec3(0.0f, 1.8f, 80.1f),
		vec3(0.0f, 1.4f, 100.1f),
		vec3(-2.2f, 2.5f, 128.8f),
		vec3(2.8f, 1.1f, 140.1f),
		vec3(1.6f, 1.4f, 160.1f),
		vec3(-0.3f, 1.4f, 180.1f),
		vec3(1.8f, 1.4f, 200.1f),
		vec3(-2.9f, 1.6f, 222.2f),
		vec3(1.5f, 1.4f, 247.5f),
		vec3(-0.7f, 1.4f, 268.0f),
		vec3(0.0f, 1.4f, 280.1f),
		vec3(-1.4f, 1.4f, 306.0f),
		vec3(0.7f, 1.4f, 326.0f),
		vec3(-7.79997f, 2.3f, 357.8f),
		vec3(22.4f, 5.2f, 360.5f),
		vec3(4.1f, 10.1f, 340.797f),
		vec3(-11.3f, 13.0f, 352.997f),
		vec3(0.0f, 14.0f, 360.696f),
		vec3(9.8f, 14.3f, 356.295f),
		vec3(4.0f, 20.3f, 340.493f)
	});
	vector<vec3> firefliesAColors = vector<vec3> ({
		vec3(0.8f, 0.2f, 0.0f),
		vec3(0.7f, 0.5f, 0.0f),
		vec3(0.4f, 0.6f, 0.1f),
		vec3(0.3f, 0.7f, 0.2f),
		vec3(0.4f, 0.5f, 0.3f)
	});
	firefliesA = new Fireflies(MAX_PARTICLES, firefliesAPathPoints, firefliesAColors);
	
	vector<vec3> firefliesBPathPoints = vector<vec3>({
		// vec3(28.2f, 8.4f, 249.2f),
		// vec3(-2.1f, 2.2f, 269.804f),
		// vec3(0.6f, 1.4f, 292.301f),
		// vec3(-1.5f, 1.79999f, 304.196f),
		// vec3(3.7f, 2.4f, 326.696f),
		// vec3(14.8f, 6.2f, 360.497f),
		// vec3(-15.2f, 11.9f, 362.397f),
		// vec3(-5.6f, 15.6f, 338.594f),
		// vec3(13.1f, 18.4f, 356.593f),
		// vec3(-10.0f, 23.3f, 360.596f),
		// vec3(-8.5f, 22.8f, 345.394f),
		// vec3(-4.2f, 27.3f, 364.394f)
		vec3(28.1f, 10.1f, 205.397f),
		vec3(-5.10007f, 1.5f, 231.598f),
		vec3(4.8f, 2.2f, 250.803f),
		vec3(0.6f, 1.4f, 273.8f),
		vec3(-1.5f, 1.79999f, 286.595f),
		vec3(2.7f, 2.4f, 304.295f),
		vec3(0.29998f, 1.3f, 323.095f),
		vec3(9.00004f, 2.49999f, 356.597f),
		vec3(-5.6f, 3.29998f, 362.995f),
		vec3(-17.5f, 12.4f, 356.493f),
		vec3(-10.0f, 23.3f, 360.596f),
		vec3(-8.5f, 22.8f, 345.394f),
		vec3(-4.2f, 27.3f, 364.394f)
	});
	vector<vec3> firefliesBColors = vector<vec3> ({
		vec3(0.4f, 0.5f, 0.3f),
		vec3(0.3f, 0.7f, 0.2f),
		vec3(0.4f, 0.6f, 0.1f),
		vec3(0.7f, 0.5f, 0.0f),
		vec3(0.8f, 0.2f, 0.0f)
	});
	firefliesB = new Fireflies(MAX_PARTICLES, firefliesBPathPoints, firefliesBColors);

	vector<vec3> phoenixPathPoints = {
		vec3(0.0f, 45.0f, 380.1f),
		vec3(8.3f, 20.7f, 400.1f),
		vec3(-3.7f, 9.89999f, 407.799f),
		vec3(1.6f, 7.4f, 440.1f),
		vec3(-5.4f, 8.4f, 460.1f),
		vec3(1.3f, 10.4f, 480.1f),
		vec3(-1.2f, 9.1f, 500.1f),
		vec3(6.1f, 11.2f, 525.0f),
		vec3(0.2f, 11.6f, 550.1f),
		vec3(0.4f, 5.4f, 575.1f),
		vec3(24.2001f, 5.1f, 603.294f),
		vec3(40.4f, 5.1f, 685.279f),
		vec3(102.8f, 14.3f, 946.12f),
		vec3(-95.1991f, 30.6f, 946.323f),
		vec3(-18.9f, 47.7f, 887.839f)
	};
	phoenixPath = new BsplineInterpolator(phoenixPathPoints);
	pathPhoenix = new SplineRenderer(phoenixPath);
	pathAdjuster = new SplineAdjuster(phoenixPath);
	pathAdjuster->setRenderPoints(true);

	iblNight = new CubeMapRenderTarget(2048,2048,false);
	iblNight->setPosition(vec3(-0.3f, -5.1f, 1157.5f));

	iblLight = new SceneLight(true);
	iblLight->addDirectionalLights({
		DirectionalLight(vec3(0.3f, 0.3f, 0.3f), 3.0f, vec3(0.0f, -1.0f, -1.0f)),
		DirectionalLight(vec3(0.3f, 0.3f, 0.3f), 3.0f, vec3(0.0f, -1.0f, 1.0f)),
	});

}

void renderForIBL(mat4 &projection,mat4& view,vec3& camPos) {

///////////////Skybox///////////////////////////////////////////////////////////
	programSkybox->use();
	glUniformMatrix4fv(programSkybox->getUniformLocation("pMat"),1,GL_FALSE,projection);
	glUniformMatrix4fv(programSkybox->getUniformLocation("vMat"),1,GL_FALSE,view);
	glUniform1i(programSkybox->getUniformLocation("skyboxColor"), 0);
	glUniform1i(programSkybox->getUniformLocation("skyboxEmmission"), 1);
	glUniform1f(programSkybox->getUniformLocation("emmissionPower"), (*nightevents)[CROSSIN_T]);
	glBindVertexArray(skybox_vao);
	glBindTextureUnit(0, texColorNightSky);
	glBindTextureUnit(1, texEmmissionNightSky);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
////////////////////////////////////////////////////////////////////////////////

///////////////Islands//////////////////////////////////////////////////////////
	programTerrain->use();
	glUniformMatrix4fv(programTerrain->getUniformLocation("pMat"), 1, GL_FALSE, projection);
	glUniformMatrix4fv(programTerrain->getUniformLocation("vMat"), 1, GL_FALSE, view);
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, 1250.0f));
	glUniform3fv(programTerrain->getUniformLocation("cameraPos"), 1, camPos);
	glUniform1f(programTerrain->getUniformLocation("maxTess"), island->getMaxTess());
	glUniform1f(programTerrain->getUniformLocation("minTess"), island->getMinTess());
	glUniform1i(programTerrain->getUniformLocation("texHeight"), 0);
	// glUniform1i(programTerrain->getUniformLocation("texNormal"), 1);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseGrass"), 2);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseDirt"), 3);
	glUniform1f(programTerrain->getUniformLocation("texScale"), 15.0f);
	glBindTextureUnit(0, island->getHeightMap());
	glBindTextureUnit(1, island->getNormalMap());
	glBindTextureUnit(2, texDiffuseGrass);
	glBindTextureUnit(3, texDiffuseDirt);
	island->render();
////////////////////////////////////////////////////////////////////////////////

///////////////Ocean////////////////////////////////////////////////////////////
	programOcean->use();
	glUniformMatrix4fv(programOcean->getUniformLocation("pMat"), 1, false, projection);
	glUniformMatrix4fv(programOcean->getUniformLocation("vMat"), 1, false, view);
	glUniformMatrix4fv(programOcean->getUniformLocation("mMat"), 1, false, translate(0.0f, -7.0f, 1300.0f) * scale(721.0f));
	glUniform3fv(programOcean->getUniformLocation("cameraPosition"), 1, camPos);
	glUniform3fv(programOcean->getUniformLocation("oceanColor"), 1, oceanColor);
	glUniform3fv(programOcean->getUniformLocation("skyColor"), 1, skyColor);
	glUniform3fv(programOcean->getUniformLocation("sunDirection"), 1, sunDirection);
	glUniform1i(programOcean->getUniformLocation("displacementMap"), 0);
	glUniform1i(programOcean->getUniformLocation("normalMap"), 1);
	glBindTextureUnit(0, obocean->getDisplacementMap());
	glBindTextureUnit(1, obocean->getNormalMap());
	obocean->render(programOcean);
////////////////////////////////////////////////////////////////////////////////

///////////////Rockets//////////////////////////////////////////////////////////
	vec3 positions[] = {
		vec3(0.0f, mix(vec1(420.1f), vec1(100.1f), (*nightevents)[ROCKET_T])[0], 774.8f),
		vec3(180.0f, mix(vec1(520.1f), vec1(70.1f), (*nightevents)[ROCKET_T])[0], 700.1f),
		vec3(190.0f, mix(vec1(800.1f), vec1(120.1f), (*nightevents)[ROCKET_T])[0], 400.1f),
		// vec3(-80.0f, mix(vec1(400.1f), vec1(100.1f), (*nightevents)[ROCKET_T])[0], 360.1f),
		vec3(-160.0f, mix(vec1(545.1f), vec1(160.1f), (*nightevents)[ROCKET_T])[0], 630.1f),
		vec3(-200.0f, mix(vec1(420.1f), vec1(190.1f), (*nightevents)[ROCKET_T])[0], 750.1f),
		// vec3(50.0f, mix(vec1(400.1f), vec1(100.1f), (*nightevents)[ROCKET_T])[0], 550.1f),
	};

	programStaticPBR->use();
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"), 1, GL_FALSE, projection);
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"), 1, GL_FALSE, view);
	glUniform3fv(programStaticPBR->getUniformLocation("viewPos"), 1, camPos);
	glUniform1i(programStaticPBR->getUniformLocation("specularGloss"), GL_FALSE);
	lightManager->setLightUniform(programStaticPBR, false);
	for(int i = 0; i < 5; i++) {
		programStaticPBR->use();
		mat4 translateMat = translate(positions[i]);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translateMat * rotate(90.0f, 1.0f, 0.0f, 0.0f) * rotate(-30.0f, 0.0f, 0.0f, 1.0f) * scale(5.5f));
		modelRocket->draw(programStaticPBR);
	
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		programFire->use();
		glUniformMatrix4fv(programFire->getUniformLocation("pMat"), 1, GL_FALSE, projection);
		glUniformMatrix4fv(programFire->getUniformLocation("vMat"), 1, GL_FALSE, view);
		glUniformMatrix4fv(programFire->getUniformLocation("mMat"), 1, GL_FALSE, translateMat * translate(-0.2f, -40.0f, -0.5f) * scale(8.3f, 20.3f, 1.0f));
		glUniform1f(programFire->getUniformLocation("time"), fireT);
		programglobal::shapeRenderer->renderQuad();
		glDisable(GL_BLEND);
	}
////////////////////////////////////////////////////////////////////////////////
}

void preOceanRender() {
	programTerrain->use();
	glUniformMatrix4fv(programTerrain->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programTerrain->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniform3fv(programTerrain->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programTerrain->getUniformLocation("texHeight"), 0);
	// glUniform1i(programTerrain->getUniformLocation("texNormal"), 1);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseGrass"), 3);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseDirt"), 4);
	glUniform1f(programTerrain->getUniformLocation("texScale"), 15.0f);
	glBindTextureUnit(3, texDiffuseGrass);
	glBindTextureUnit(4, texDiffuseDirt);
	lightManager->setLightUniform(programTerrain, false);

	glBindTextureUnit(0, land->getHeightMap());
	glBindTextureUnit(1, land->getNormalMap());
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, scale(6.0f));
	glUniform1f(programTerrain->getUniformLocation("maxTess"), land->getMaxTess());
	glUniform1f(programTerrain->getUniformLocation("minTess"), land->getMinTess());
	land->render();
	
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, 64.0f * 6.0f - 1.0f) * scale(6.0f));
	glUniform1f(programTerrain->getUniformLocation("maxTess"), land2->getMaxTess());
	glUniform1f(programTerrain->getUniformLocation("minTess"), land2->getMinTess());
	glBindTextureUnit(0, land2->getHeightMap());
	glBindTextureUnit(1, land2->getNormalMap());
	land2->render();
	
	for(int i = 0; i < 9; i++) {
		glBindTextureUnit(i, 0);
	}
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
			count = std::min(420 - n, 90);
			glUniform1i(programStaticInstancedPBR->getUniformLocation("instanceOffset"), n);
		} else {
			glUniform1i(programStaticInstancedPBR->getUniformLocation("instanceOffset"), 0);
		}
		if(count > 0) {
			modelTreeRed->draw(programStaticInstancedPBR, count);
		}

		// count = 200;
		// if(z >= -95.0f) {
		// 	//z += 95.0f;
		// 	int n = 8 * (int)(z / 10.0f);
		// 	count = std::min(420 - n, 200);
		// 	glUniform1i(programStaticInstancedPBR->getUniformLocation("instanceOffset"), n);
		// } else {
		// 	glUniform1i(programStaticInstancedPBR->getUniformLocation("instanceOffset"), 0);
		// }
		// glUniformMatrix4fv(programStaticInstancedPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.3f, 0.0f) * rotate(0.0f,vec3(1.0f,0.0f,0.0f)) * scale(1.0f));
		// glUniform3fv(programStaticInstanc1.0 / resolution.xyedPBR->getUniformLocation("emissionColor"),1,vec3(0.0,0.0,0.0));
		// glBindBufferBase(GL_UNIFORM_BUFFER, programStaticInstancedPBR->getUniformLocation("position_ubo"), uboFlowerPosition);
		// glBindBufferBase(GL_UNIFORM_BUFFER, programStaticInstancedPBR->getUniformLocation("color_ubo"), uboTreeColor);
		// modelFlowerPurple->draw(programStaticInstancedPBR, count);
	}

	programDynamicPBR->use();
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"), GL_FALSE);
	lightManager->setLightUniform(programDynamicPBR, false);
	modelFox->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(-16.8f, 0.03f, mix(vec1(-91.0828f), vec1(-84.0828f), (*nightevents)[FOXWALK_T])[0]) * rotate(-77.0f, 0.0f, 1.0f, 0.0f) * scale(0.036f));
	modelFox->draw(programDynamicPBR);
}

void postOceanRender() {

	if(iblSetup){
		glBindFramebuffer(GL_FRAMEBUFFER,iblNight->FBO);
		glViewport(0, 0, iblNight->width, iblNight->height);
		for(int side = 0; side < 6; side++)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + side,iblNight->cubemap_texture,0);
			glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.1f, 0.1f, 1.0f));
			glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
			renderForIBL(iblNight->projection,iblNight->view[side],iblNight->position);
		}
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		iblLight->setEnvmap(iblNight->cubemap_texture);
		iblLight->PrecomputeIndirectLighting();
		iblSetup = false;
	}

	programTerrain->use();
	glUniformMatrix4fv(programTerrain->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programTerrain->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, 1250.0f));
	glUniform3fv(programTerrain->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1f(programTerrain->getUniformLocation("maxTess"), island->getMaxTess());
	glUniform1f(programTerrain->getUniformLocation("minTess"), island->getMinTess());
	glUniform1i(programTerrain->getUniformLocation("texHeight"), 0);
	// glUniform1i(programTerrain->getUniformLocation("texNormal"), 1);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseGrass"), 2);
	glUniform1i(programTerrain->getUniformLocation("texDiffuseDirt"), 3);
	glUniform1f(programTerrain->getUniformLocation("texScale"), 15.0f);
	glBindTextureUnit(0, island->getHeightMap());
	glBindTextureUnit(1, island->getNormalMap());
	glBindTextureUnit(2, texDiffuseGrass);
	glBindTextureUnit(3, texDiffuseDirt);
	island->render();

	programDynamicPBR->use();
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"), GL_FALSE);
	iblLight->setLightUniform(programDynamicPBR);
	modelDrone->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(2.3f, -1.0f, 1161.0f) * rotate(180.0f, 0.0f, 1.0f, 0.0f) * scale(15.0f));
	modelDrone->draw(programDynamicPBR);

	modelAstro->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
	glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(-0.3f, -5.1f, 1157.5f) * rotate(180.0f, 0.0f, 1.0f, 0.0f) * scale(3.0f));
	modelAstro->draw(programDynamicPBR);

	programStaticPBR->use();
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniform3fv(programStaticPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
	glUniform1i(programStaticPBR->getUniformLocation("specularGloss"), GL_FALSE);
	iblLight->setLightUniform(programStaticPBR);
	glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(-6.0f, 5.5f, 1186.0f) * rotate(90.0f, 1.0f, 0.0f, 0.0f) * rotate(180.0f, 0.0f, 1.0f, 0.0f) * scale(2.0f));
	modelTie->draw(programStaticPBR);

	if((*nightevents)[ROCKET_T] > 0.0f) {
		vec3 positions[] = {
			vec3(0.0f, mix(vec1(420.1f), vec1(100.1f), (*nightevents)[ROCKET_T])[0], 774.8f),
			vec3(180.0f, mix(vec1(520.1f), vec1(70.1f), (*nightevents)[ROCKET_T])[0], 700.1f),
			vec3(190.0f, mix(vec1(800.1f), vec1(120.1f), (*nightevents)[ROCKET_T])[0], 400.1f),
			// vec3(-80.0f, mix(vec1(400.1f), vec1(100.1f), (*nightevents)[ROCKET_T])[0], 360.1f),
			vec3(-160.0f, mix(vec1(545.1f), vec1(160.1f), (*nightevents)[ROCKET_T])[0], 630.1f),
			vec3(-200.0f, mix(vec1(420.1f), vec1(190.1f), (*nightevents)[ROCKET_T])[0], 750.1f),
			// vec3(50.0f, mix(vec1(400.1f), vec1(100.1f), (*nightevents)[ROCKET_T])[0], 550.1f),
		};


		for(int i = 0; i < 5; i++) {
			programStaticPBR->use();
			mat4 translateMat = translate(positions[i]);
			glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translateMat * rotate(90.0f, 1.0f, 0.0f, 0.0f) * rotate(-30.0f, 0.0f, 0.0f, 1.0f) * scale(5.5f));
			modelRocket->draw(programStaticPBR);
		
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			programFire->use();
			glUniformMatrix4fv(programFire->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
			glUniformMatrix4fv(programFire->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
			glUniformMatrix4fv(programFire->getUniformLocation("mMat"), 1, GL_FALSE, translateMat * translate(-0.2f, -40.0f, -0.5f) * scale(8.3f, 20.3f, 1.0f));
			glUniform1f(programFire->getUniformLocation("time"), fireT);
			programglobal::shapeRenderer->renderQuad();
			glDisable(GL_BLEND);
		}
	}
}

void nightscene::render() {
	camera1->setT((*nightevents)[CAMERAMOVE1_T]);
	camera2->setT((*nightevents)[CAMERAMOVE2_T]);

	try {
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

	if((*nightevents)[CAMERAMOVE1_T] < 0.81) {
		preOceanRender();
	}
	if((*nightevents)[CAMERAMOVE1_T] > 0.6) {
		postOceanRender();
	}

	programOcean->use();
	glUniformMatrix4fv(programOcean->getUniformLocation("pMat"), 1, false, programglobal::perspective);
	glUniformMatrix4fv(programOcean->getUniformLocation("vMat"), 1, false, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programOcean->getUniformLocation("mMat"), 1, false, translate(0.0f, -7.0f, 1300.0f) * scale(721.0f));
	glUniform3fv(programOcean->getUniformLocation("cameraPosition"), 1, programglobal::currentCamera->position());
	glUniform3fv(programOcean->getUniformLocation("oceanColor"), 1, oceanColor);
	glUniform3fv(programOcean->getUniformLocation("skyColor"), 1, skyColor);
	glUniform3fv(programOcean->getUniformLocation("sunDirection"), 1, sunDirection);
	glUniform1i(programOcean->getUniformLocation("displacementMap"), 0);
	glUniform1i(programOcean->getUniformLocation("normalMap"), 1);
	glBindTextureUnit(0, obocean->getDisplacementMap());
	glBindTextureUnit(1, obocean->getNormalMap());
	obocean->render(programOcean);

	if((*nightevents)[FIREFLIES1BEGIN_T] > 0.0f && (*nightevents)[FIREFLIES1BEGIN_T] < 1.0f) {
		firefliesA->renderAsSpheres((*nightevents)[FIREFLIES1BEGIN_T], 0.05f);
		// firefliesA->renderAttractorAsQuad((*nightevents)[FIREFLIES1BEGIN_T], 0.25f);
	}
	if((*nightevents)[FIREFLIES2BEGIN_T] > 0.0f && (*nightevents)[FIREFLIES2BEGIN_T] < 1.0f) {
		firefliesB->renderAsSpheres((*nightevents)[FIREFLIES2BEGIN_T], 0.05f);
		// firefliesB->renderAttractorAsQuad((*nightevents)[FIREFLIES2BEGIN_T], 0.25f);
	}

	if((*nightevents)[PHOENIXFLY_T] > 0.0f && (*nightevents)[PHOENIXFLY_T] < 1.0f) {
		programDynamicPBR->use();
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix());
		modelPhoenix->update(0.01f, 0);
		modelPhoenix->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
		glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"),1,programglobal::currentCamera->position());
		glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"),false);
		lightManager->setLightUniform(programDynamicPBR, false);
		vec3 phoenixPosition = phoenixPath->interpolate((*nightevents)[PHOENIXFLY_T] - 0.01f);
		vec3 phoenixFront = phoenixPath->interpolate((*nightevents)[PHOENIXFLY_T]);
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"),1,GL_FALSE, translate(phoenixPosition) * targetat(phoenixPosition, phoenixFront, vec3(0.0f, 1.0f, 0.0f)) * rotate(-90.0f,vec3(0.0f,1.0f,0.0f)) * scale(10.0f,10.0f,10.0f));
		modelPhoenix->draw(programDynamicPBR);
	}

	if((*nightevents)[CAMERAMOVE1_T] > 0.136f && (*nightevents)[CAMERAMOVE1_T] < 0.26f) {
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
	}

	if((*nightevents)[CROSSIN_T] < 1.0f) {
		crossfader::render(texDaySceneFinal, (*nightevents)[CROSSIN_T]);
	}

	if(programglobal::debugMode == CAMERA) {
		camRig1->render();
	} else if(programglobal::debugMode == SPLINE) {
		pathAdjuster->render(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f));
	} else if(programglobal::debugMode == LIGHT) {
		lightManager->renderSceneLights();
	}
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void nightscene::update() {
	static const float DRONE_ANIM_SPEED = 0.8f;
	static const float ASTRO_ANIM_SPEED = 0.5f;
	static const float OCEAN_ANIM_SPEED = 0.2f;
	static const float PHOENIX_ANIM_SPEED = 0.6f;
	static const float WOLF_ANIM_SPEED = 0.8f;
	static const float FIRE_ANIM_SPEED = 0.4f;
	
	nightevents->increment();
	if((*nightevents)[CROSSIN_T] >= 0.01f) {
		if(programglobal::isAnimating) {
			playerBkgnd->play();
		} else {
			playerBkgnd->pause();
		}
	}

	if((*nightevents)[FADEOUT_T] >= 1.0f) {
		playNextScene();
	}

	modelDrone->update(DRONE_ANIM_SPEED * programglobal::deltaTime, 1);
	modelAstro->update(ASTRO_ANIM_SPEED * programglobal::deltaTime, 1);
	modelPhoenix->update(PHOENIX_ANIM_SPEED * programglobal::deltaTime, 0);
	modelFox->update(WOLF_ANIM_SPEED * programglobal::deltaTime, 0);
	obocean->update(OCEAN_ANIM_SPEED * programglobal::deltaTime);
	firefliesA->update();
	firefliesB->update();
	fireT += FIRE_ANIM_SPEED * programglobal::deltaTime;
}

void nightscene::reset() {
	nightevents->resetT();
	playerBkgnd->restart();
}

void nightscene::uninit() {
	delete land;
	delete pathAdjuster;
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
	} else if(programglobal::debugMode == LIGHT) {
		lightManager->SceneLightKeyBoardFunc(key);
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
	case XK_F10:
		// delete pathAdjuster;
		// pathAdjuster = new SplineAdjuster(firefliesA->getPath());
		// pathAdjuster->setScalingFactor(0.1f);
		// pathAdjuster->setRenderPoints(true);
		break;
	case XK_F11:
		// delete pathAdjuster;
		// pathAdjuster = new SplineAdjuster(firefliesB->getPath());
		// pathAdjuster->setScalingFactor(0.1f);
		// pathAdjuster->setRenderPoints(true);
		break;
	case XK_F12:
		renderTrees = !renderTrees;
		break;
	case XK_Tab:
		if(programglobal::debugMode == CAMERA) {
			cout<<camRig1->getCamera()<<endl;
		} else if(programglobal::debugMode == SPLINE) {
			cout<<pathAdjuster->getSpline() << endl;
		} else if(programglobal::debugMode == MODEL) {
			cout<<quickModelPlacer<<endl;
		}
		cout<<nightevents->getT()<<endl;
		break;
	}
}

camera* nightscene::getCamera() {
	if((*nightevents)[CAMERAMOVE1_T] >= 1.0f) {
		return camera2;
	} else {
		return camera1;
	}
}

void nightscene::crossfade() {
	if((*nightevents)[CAMERAMOVE1_T] > 0.136f && (*nightevents)[CAMERAMOVE1_T] < 0.26f) {
		godraysMoon->renderRays();
	}
	if((*nightevents)[FADEOUT_T] > 0.0f) {
		crossfader::render(texBlack, 1.0f - (*nightevents)[FADEOUT_T]);
	}
}