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

using namespace std;
using namespace vmath;

static glshaderprogram* programTerrain;
static glshaderprogram* programStaticPBR;
static glshaderprogram* programDynamicPBR;

static terrain* land;
static terrain* land2;

static SceneLight* lightManager;

static sceneCamera* camera1;

static sphere* sphereMap;

#ifdef DEBUG
static sceneCameraRig* camRig1;
static bool renderPath = false;
static audioplayer* playerBkgnd;
#endif

extern GLuint texDaySceneFinal;
static GLuint texDiffuseGrass;
static GLuint texDiffuseDirt;
	
enum tvalues {
	CROSSIN_T,
	CAMERAMOVE_T
};
static eventmanager* nightevents;

void nightscene::setupProgram() {
	try {
		programTerrain = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/renderjungle.frag"});
		programStaticPBR = new glshaderprogram({"shaders/pbrStatic.vert", "shaders/pbrMain.frag"});
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
	} catch(string errorString)  {
		throwErr(errorString);
	}
}

void nightscene::setupCamera() {
	vector<vec3> positionVector = {
		vec3(0.0f, 5.61237f, -122.191f),
		vec3(0.0f, 5.61237f, -121.891f),
		vec3(0.0f, 5.61237f, -120.691f),
		vec3(0.0f, 5.61237f, -109.391f)
	};
	vector<vec3> frontVector = {
		vec3(0.0f, 12.21f, -109.7f),
		vec3(0.0f, 9.21f, -105.09f),
		vec3(0.0f, 6.0f, -98.29f),
		vec3(0.0f, 5.5f, -89.19f)
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
		{CROSSIN_T, { 0.0f, 1.0f }},
		{CAMERAMOVE_T, { 1.0f, 8.0f }}
	});

	texDiffuseGrass = createTexture2D("resources/textures/grass.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	texDiffuseDirt = createTexture2D("resources/textures/dirt.png", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	GLuint texJungle = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 0), 256.0f, 5.0f, 5, 235);
	GLuint texJungle2 = opensimplexnoise::createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 1024), 256.0f, 5.0f, 5, 235);

	sphereMap = new sphere(25, 50, 1.0f);
	land = new terrain(texJungle, 256, true, 5, 16);
	land2 = new terrain(texJungle2, 256, true, 5, 16);
	lightManager = new SceneLight(false);
	lightManager->addDirectionalLight(DirectionalLight(vec3(1.0f, 1.0f, 1.0f), 10.0f, vec3(0.0f, -1.0f, 0.0f)));
}

void nightscene::render() {
	camera1->setT((*nightevents)[CAMERAMOVE_T]);

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
		} else 
		if(programglobal::debugMode == SPLINE) {
		}
		if(programglobal::debugMode == MODEL) {
		}
		break;
	}
}

camera* nightscene::getCamera() {
	return camera1;
}

void nightscene::crossfade() {
	if((*nightevents)[CROSSIN_T] < 1.0f) {
		crossfader::render(texDaySceneFinal, (*nightevents)[CROSSIN_T]);
	}
}