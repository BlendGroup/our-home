#include<iostream>
#include<memory>

#include<GL/glew.h>
#include<GL/gl.h>

#include<vmath.h>
#include<glshaderloader.h>
#include<scenecamera.h>
#include<debugcamera.h>
#include<testPBR.h>
#include<testLab.h>
#include<testmodel.h>
#include<hdr.h>
#include<windowing.h>
#include<errorlog.h>
#include<global.h>
#include<clhelper.h>
#include<opensimplexnoise.h>

#include<testeffect.h>
#include<testcamera.h>
#include<testmodel.h>
#include<testterrain.h>
#include<testcubemap.h>
#include<testnoise.h>

using namespace std;
using namespace vmath;

static bool hdrEnabled = false;
static HDR* hdr;
static sceneCamera* scenecamera;
static sceneCameraRig* scenecamerarig;
static debugCamera *debugcamera;
static bool isDebugCameraOn = false;
static bool isAnimating = false;

#define SHOW_TEST_SCENE 		0
#define SHOW_MODEL_SCENE 		0
#define SHOW_CAMERA_SCENE 		1
#define SHOW_PBR_SCENE			0
#define SHOW_LAB_SCENE			0
#define SHOW_CAMERA_RIG			1
#define SHOW_TERRAIN_SCENE 		0
#define SHOW_CUBEMAP_SCENE		0
#define SHOW_NOISE_SCENE 		0

mat4 programglobal::perspective;
clglcontext* programglobal::oclContext;
camera* programglobal::currentCamera;
opensimplexnoise* programglobal::noiseGenerator;

extern vector<vec3> positionKeyFrames;
extern vector<vec3> frontKeyFrames;

void setupProgram(void) {
	try {
		programglobal::oclContext->compilePrograms({"shaders/terrain/calcnormals.cl"});
	
#if SHOW_TEST_SCENE
		setupProgramTestEffect();
#endif
#if SHOW_CAMERA_SCENE
		setupProgramTestCamera();
#endif
#if SHOW_MODEL_SCENE
		setupProgramTestModel();
#endif
#if SHOW_PBR_SCENE
		setupProgramTestPbr();
#endif
#if SHOW_LAB_SCENE
		setupProgramTestLab();
#endif
#if SHOW_TERRAIN_SCENE
		setupProgramTestTerrain();
#endif
#if SHOW_CUBEMAP_SCENE
		setupProgramTestRenderToCubemap();
#endif
#if SHOW_NOISE_SCENE
		setupProgramTestNoise();
#endif

		hdr->setupProgram();
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void setupSceneCamera(void) {
	try {
		debugcamera = new debugCamera(vec3(0.0f, 5.0f, 5.0f), -90.0f, 0.0f);
#if SHOW_CAMERA_RIG
		setupSceneCameraRigTestCamera(scenecamerarig);
#else
		setupSceneCameraTestCamera(scenecamera);
#endif // SHOW_CAMERA_RIG
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void init(void) {
	try {
		//Object Creation
		hdr = new HDR(1.5f, 1.0f, 2048);
		programglobal::oclContext = new clglcontext(1);
		programglobal::noiseGenerator = new opensimplexnoise();	

		//Inititalize
#if SHOW_TEST_SCENE
		initTestEffect();
#endif
#if SHOW_CAMERA_SCENE
		initTestCamera();
#endif
#if SHOW_MODEL_SCENE
		initTestModel();
#endif
#if SHOW_PBR_SCENE
		initTestPbr();
#endif
#if SHOW_LAB_SCENE
	initTestLab();
#endif
#if SHOW_TERRAIN_SCENE
		initTestTerrain();
#endif
#if SHOW_CUBEMAP_SCENE
		initTestRenderToCubemap();
#endif
#if SHOW_NOISE_SCENE
		initTestNoise();
#endif
		hdr->init();

		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void render(glwindow* window) {
	try {
#if SHOW_CAMERA_RIG
		programglobal::currentCamera = isDebugCameraOn ? dynamic_cast<camera*>(debugcamera) : dynamic_cast<camera*>(scenecamerarig->getCamera());
#else
		programglobal::currentCamera = isDebugCameraOn ? dynamic_cast<camera*>(debugcamera) : dynamic_cast<camera*>(scenecamera);
#endif
		if(hdrEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER, hdr->getFBO());
			glViewport(0, 0, hdr->getSize(), hdr->getSize());
		} else {
			glViewport(0, 0, window->getSize().width, window->getSize().height);
		}

		glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.3f, 0.2f, 1.0f));
		glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
		programglobal::perspective = perspective(45.0f, window->getSize().width / window->getSize().height, 0.1f, 1000.0f);

#if SHOW_TEST_SCENE
		renderTestEffect();
#endif
#if SHOW_CAMERA_SCENE
#if SHOW_CAMERA_RIG
		renderCameraRigTestCamera(scenecamerarig);
#endif // SHOW_CAMERA_RIG
		renderTestCamera();
#endif // SHOW_CAMERA_SCENE
#if SHOW_MODEL_SCENE
		renderTestModel(dynamic_cast<camera*>(debugcamera));
#endif
#if SHOW_PBR_SCENE
		renderTestPbr(dynamic_cast<camera*>(debugcamera),debugcamera->getPosition());
#endif
#if SHOW_LAB_SCENE
	renderTestLab(dynamic_cast<camera*>(debugcamera), debugcamera->position());
#endif
		// renderTestEffect();
#if SHOW_TERRAIN_SCENE
		renderTestTerrain();
#endif
#if SHOW_CUBEMAP_SCENE
		renderTestRenderToCubemap(dynamic_cast<camera*>(debugcamera));
#endif
#if SHOW_NOISE_SCENE
		renderTestNoise();
#endif

		if(hdrEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.1f, 0.1f, 1.0f));
			glViewport(0, 0, window->getSize().width, window->getSize().height);
			hdr->render();
		}
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void update(void) {
#if SHOW_CAMERA_RIG
	scenecamerarig->updateT(0.0005f);
#else
	scenecamera->updateT(0.0005f);
#endif // SHOW_CAMERA_RIG
}

void keyboard(glwindow* window, int key) {
	switch(key) {
	case XK_Escape:
		window->close();
		break;
	case XK_F1:
		window->toggleFullscreen();
		break;
	case XK_F2:
		isDebugCameraOn = !isDebugCameraOn;
		break;
	case XK_F3:
		hdrEnabled = !hdrEnabled;
		break;
	case XK_space:
		isAnimating = !isAnimating;
		break;
	}
	hdr->keyboardfunc(key);
	debugcamera->keyboardFunc(key);
#if SHOW_CAMERA_RIG
	scenecamerarig->keyboardfunc(key);
#endif
#if SHOW_TERRAIN_SCENE
	keyboardFuncTestTerrain(key);
#endif
#if SHOW_CUBEMAP_SCENE
	keyboardFuncTestRenderToCubemap(key);
#endif

}

void mouse(glwindow* window, int button, int action, int x, int y) {
	if(button == Button1) {
		debugcamera->mouseFunc(action, x, y);
	}
}

void uninit(void) {
#if SHOW_TEST_SCENE
	uninitTestEffect();
#endif
#if SHOW_CAMERA_SCENE
#if SHOW_CAMERA_RIG
	if(scenecamerarig) {
		delete scenecamerarig;
	}
#endif // SHOW_CAMERA_RIG
	if(scenecamera) {
		delete scenecamera;
	}
	uninitTestCamera();
#endif // SHOW_CAMERA_SCENE
#if SHOW_MODEL_SCENE
	uninitTestModel();
#endif
#if SHOW_PBR_SCENE
	uninitTestPbr();
#endif
#if SHOW_LAB_SCENE
	uninitTestLab();
#endif
#if SHOW_TERRAIN_SCENE
	uninitTestTerrain();
#endif
#if SHOW_CUBEMAP_SCENE
	uninitTestRenderToCubemap();
#endif
	hdr->uninit();

	delete programglobal::oclContext;
	delete hdr;

	if(debugcamera) {
		delete debugcamera;
		debugcamera = NULL;
	}
}

int main(int argc, char **argv) {
	try {
		glwindow* window = new glwindow("Our Planet", 0, 0, 1920, 1080, 460);
		init();
		setupProgram();
		setupSceneCamera();
		window->setKeyboardFunc(keyboard);
		window->setMouseFunc(mouse);
		window->setFullscreen(true);
		while(!window->isClosed()) {
			window->processEvents();
			render(window);
			if(isAnimating) {
				update();
			}
			window->swapBuffers();
		}
		uninit();
		delete window;
	} catch(string errorString) {
#ifdef DEBUG
		cerr<<errorString;
#endif
	}
	return 0;
}