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

static terrain* land;
static terrain* land2;

static SceneLight* lightManager;

static sceneCamera* camera1;

static sphere* sphereMap;

static modelplacer* quickModelPlacer;

static glmodel* modelTreeRed;

#ifdef DEBUG
static sceneCameraRig* camRig1;
static bool renderPath = false;
static audioplayer* playerBkgnd;
#endif

extern GLuint texDaySceneFinal;
static GLuint texDiffuseGrass;
static GLuint texDiffuseDirt;
static GLuint uboTreePosition;
static GLuint vaoNightSky;
static GLuint vboNightSky;
static GLuint fboNightSky;
static GLuint texColorNightSky;
static GLuint texEmmissionNightSky;
static GLuint skybox_vao;
static GLuint vbo;

enum tvalues {
	CROSSIN_T,
	CAMERAMOVE_T,
	FIREFLIES1BEGIN_T
};
static eventmanager* nightevents;

static const int MAX_PARTICLES = 256;
static Flock *firefliesA = NULL;
static Flock *fireflies2B = NULL;
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
		programStaticInstancedPBR = new glshaderprogram({"shaders/pbrStaticInstanced.vert", "shaders/pbrMain.frag"});
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
		programSkybox = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/nightsky/rendercubemap.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void nightscene::setupCamera() {
	vector<vec3> positionVector = {
		vec3(0.0f, 1.4f, -122.191f),
		vec3(0.0f, 1.4f, -121.891f),
		vec3(0.0f, 1.4f, -120.691f),
		vec3(0.0f, 1.4f, -117.691f),
		vec3(0.0f, 1.4f, -114.091f),
		vec3(0.0f, 1.4f, -107.891f),
		vec3(0.0f, 1.4f, -99.3911f),
		vec3(0.0f, 1.4f, -90.6912f)
	};
	vector<vec3> frontVector = {
		vec3(0.0f, 23.81f, -109.7f),
		vec3(0.0f, 12.51f, -107.89f),
		vec3(0.0f, 3.2f, -102.99f),
		vec3(0.0f, 1.4f, -96.3899f),
		vec3(0.0f, 1.4f, -92.09f),
		vec3(0.0f, 1.4f, -85.8901f),
		vec3(0.0f, 1.4f, -79.2902f),
		vec3(0.0f, 1.4f, -73.5903f)
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
		{CAMERAMOVE_T, { 2.0f, 8.0f }},
		{FIREFLIES1BEGIN_T, {8.0f, 30.0f}}
	});

	float startz = -105.0f;
	float dz = 10.0f;
	float startx = -25.0f;
	float dx = 10.0f;
	vector<vec4> treePositionsArray;
	
	int k = 0;
	for(int i = 0; i < 10; i++) {
		for(int j = 0; j < 6; j++) {
			treePositionsArray.push_back(vec4(startx + dx * j + programglobal::randgen->getRandomFloat(-4.0f, 4.0f), 0.0f, startz + dz * i + programglobal::randgen->getRandomFloat(-4.0f, 4.0f), 0.0f));
		}
	}

	texDiffuseGrass = createTexture2D("resources/textures/grass.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseDirt = createTexture2D("resources/textures/dirt.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	GLuint texJungle = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 0), 256.0f, 2.0f, 5, 235);
	GLuint texJungle2 = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 1024), 256.0f, 2.0f, 5, 235);

	modelTreeRed = new glmodel("resources/models/tree/wtree.glb", aiProcessPreset_TargetRealtime_Quality, true);
	
	glGenBuffers(1, &uboTreePosition);
	glBindBuffer(GL_UNIFORM_BUFFER, uboTreePosition);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4) * 60, treePositionsArray.data(), GL_STATIC_DRAW);
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

	sphereMap = new sphere(25, 50, 1.0f);
	land = new terrain(texJungle, 256, true, 5, 16);
	land2 = new terrain(texJungle2, 256, true, 5, 16);
	quickModelPlacer = new modelplacer();
	lightManager = new SceneLight(false);
	lightManager->addDirectionalLight(DirectionalLight(vec3(1.0f, 1.0f, 1.0f), 1.0f, vec3(0.0f, -1.0f, 0.0f)));

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
	fireflies2B = new Flock(MAX_PARTICLES, attractorPositionB);

	vector<vec3> firefliesAPath1Points = vector<vec3>({
		vec3(-24.5204f, 2.3876f, -69.3911f),
		vec3(-24.5204f, 2.3876f, -69.3911f),
		vec3(-24.5204f, 2.3876f, -69.3911f),
		vec3(-0.0203418f, 1.41239f, -73.5925f),
		vec3(-9.50299f, 16.9127f, -65.7925f),
		vec3(-12.303f, 14.1127f, -28.893f),
		vec3(-58.2204f, 8.5876f, -39.2911f),
		vec3(-24.5204f, 2.3876f, -69.3911f),
		vec3(-0.0203418f, 1.41239f, -73.5925f)
	});
	firefliesAPath1 = new BsplineInterpolator(firefliesAPath1Points);
	pathA1 = new SplineRenderer(firefliesAPath1);
	pathAdjuster = new SplineAdjuster(firefliesAPath1);
	pathAdjuster->setScalingFactor(0.1f);
	pathAdjuster->setRenderPoints(true);
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
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, mat4::identity());
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
	glUniformMatrix4fv(programTerrain->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, 256.0f));
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

	programStaticInstancedPBR->use();
	glUniformMatrix4fv(programStaticInstancedPBR->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(programStaticInstancedPBR->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(programStaticInstancedPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 5.0f, 0.0f) * rotate(90.0f,vec3(1.0f,0.0f,0.0f)) * scale(1.0f));
	lightManager->setLightUniform(programStaticInstancedPBR, false);
	glBindBufferBase(GL_UNIFORM_BUFFER, programStaticInstancedPBR->getUniformLocation("position_ubo"), uboTreePosition);
	// modelTreeRed->draw(programStaticInstancedPBR, 60);

	firefliesA->renderAsSpheres(vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f), 0.05f);
	firefliesA->renderAttractorAsQuad(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.25f);

	// fireflies2B->renderAsSpheres(vec4(1.0f, 0.0f, 0.0f, 0.0f), vec4(1.0f, 0.0f, 0.0f, 0.0f), 0.08f);
	// fireflies2B->renderAttractorAsQuad(vec4(1.0f, 0.0f, 0.0f, 1.0f), vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.25f);

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
	fireflies2B->update();
	fireflies2B->setAttractorPosition(attractorPositionB);
}

void nightscene::reset() {
	nightevents->resetT();
}

void nightscene::uninit() {
	delete land;
	delete pathAdjuster;
	delete pathA1;
	delete firefliesAPath1;
	delete fireflies2B;
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
}