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
	CAMERAMOVE_T
};
static eventmanager* nightevents;

void nightscene::setupProgram() {
	try {
		programTerrain = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/renderjungle.frag"});
		programStaticPBR = new glshaderprogram({"shaders/pbrStatic.vert", "shaders/pbrMain.frag"});
		programStaticInstancedPBR = new glshaderprogram({"shaders/pbrStaticInstanced.vert", "shaders/pbrMain.frag"});
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
		programSkybox = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/debug/rendercubemap.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void nightscene::setupCamera() {
	vector<vec3> positionVector = {
		vec3(0.0f, 1.11237f, -122.191f),
		vec3(0.0f, 1.11237f, -121.891f),
		vec3(0.0f, 1.11237f, -120.691f),
		vec3(0.0f, 1.11237f, -109.391f)
	};
	vector<vec3> frontVector = {
		vec3(0.0f, 17.81f, -109.7f),
		vec3(0.0f, 10.61f, -105.09f),
		vec3(0.0f, 2.9f, -98.29f),
		vec3(0.0f, 1.6f, -89.19f)
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

float randInRange(float min, float max) {
	return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
}

void nightscene::init() {
	nightevents = new eventmanager({
		{CROSSIN_T, { 0.0f, 10.0f }},
		{CAMERAMOVE_T, { 10.0f, 8.0f }}
	});

	vec4 treePositionsArray[] = {
		vec4(14.0f, 0.0f, -97.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.0f, 0.0f)
	};

	texDiffuseGrass = createTexture2D("resources/textures/grass.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseDirt = createTexture2D("resources/textures/dirt.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	GLuint texJungle = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 0), 256.0f, 2.0f, 5, 235);
	GLuint texJungle2 = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 1024), 256.0f, 2.0f, 5, 235);

	modelTreeRed = new glmodel("resources/models/tree/redtree.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	
	glGenBuffers(1, &uboTreePosition);
	glBindBuffer(GL_UNIFORM_BUFFER, uboTreePosition);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(vec4) * 10, treePositionsArray, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	vector<float> starArray;
	for(int i = 0; i < 300; i++) {
		starArray.push_back(randInRange(-1.0f, 1.0f));
		starArray.push_back(randInRange(-1.0f, 1.0f));
		starArray.push_back(randInRange(5.0f, 20.0f));
		starArray.push_back(randInRange(0.9f, 1.0f));
		starArray.push_back(randInRange(0.6f, 1.0f));
		starArray.push_back(randInRange(0.5f, 1.0f));
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
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
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
	lightManager->addDirectionalLight(DirectionalLight(vec3(1.0f, 1.0f, 1.0f), 10.0f, vec3(0.0f, -1.0f, 0.0f)));

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
}

void nightscene::render() {
	camera1->setT((*nightevents)[CAMERAMOVE_T]);

	programSkybox->use();
	glUniformMatrix4fv(programSkybox->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
	glUniformMatrix4fv(programSkybox->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix());
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
	glUniformMatrix4fv(programStaticInstancedPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 6.0f, 0.0f) * scale(2.85f));
	lightManager->setLightUniform(programStaticInstancedPBR, false);
	glBindBufferBase(GL_UNIFORM_BUFFER, programStaticInstancedPBR->getUniformLocation("position_ubo"), uboTreePosition);
	modelTreeRed->draw(programStaticInstancedPBR, 10);

	if((*nightevents)[CROSSIN_T] < 1.0f) {
		crossfader::render(texDaySceneFinal, (*nightevents)[CROSSIN_T]);
	}

	if(programglobal::debugMode == CAMERA) {
		camRig1->render();
	} else if(programglobal::debugMode == SPLINE) {
	}
}

void nightscene::update() {
	nightevents->increment();
}

void nightscene::reset() {
	nightevents->resetT();
}

void nightscene::uninit() {
	delete land;
}

void nightscene::keyboardfunc(int key) {
	if(programglobal::debugMode == MODEL) {
		quickModelPlacer->keyboardfunc(key);
	} else if(programglobal::debugMode == CAMERA) {
		camRig1->keyboardfunc(key);
	} else if(programglobal::debugMode == SPLINE) {
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