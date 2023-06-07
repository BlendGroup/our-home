#define DEBUG
#include<scenes/night.h>
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
#include<scenecamera.h>

using namespace std;
using namespace vmath;

static glshaderprogram* programTerrain;
static glshaderprogram* programStaticPBR;
static glshaderprogram* programDynamicPBR;

static terrain* land;

static SceneLight* lightManager;

static sceneCamera* camera1;

#ifdef DEBUG
static sceneCameraRig* camRig1;
static bool renderPath = false;
static audioplayer* playerBkgnd;
#endif

extern GLuint texDaySceneFinal;
	
enum tvalues {
	CROSSIN_T,
};
static eventmanager* nightevents;

void nightscene::setupProgram() {
	try {
		programTerrain = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/render.frag"});
		programStaticPBR = new glshaderprogram({"shaders/pbrStatic.vert", "shaders/pbrMain.frag"});
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
	} catch(string errorString)  {
		throwErr(errorString);
	}
}

void nightscene::setupCamera() {
	vector<vec3> positionVector = {
		vec3(-36.3205f, 14.7124f, -130.091f),
		vec3(-71.8202f, 16.6124f, -130.991f),
		vec3(-82.7206f, 16.1124f, -122.591f),
		vec3(-86.9206f, 18.4124f, -109.391f)
	};
	vector<vec3> frontVector = {
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
}
void nightscene::init() {
	nightevents = new eventmanager({
		{CROSSIN_T, { 0.0f, 1.0f }},
	});
}

void nightscene::render() {
	// camera1->setT((*dayevents)[CAMERA1MOVE_T]);

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

	// }
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