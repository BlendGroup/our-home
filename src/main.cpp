#include<iostream>

#include<GL/glew.h>
#include<GL/gl.h>

#include"../include/vmath.h"
#include"../include/glshaderloader.h"
#include"../include/testeffect.h"
#include"../include/testcamera.h"
#include"../include/scenecamera.h"
#include"../include/debugcamera.h"
#include"../include/testmodel.h"
#include"../include/hdr.h"
#include"../include/windowing.h"
#include"../include/errorlog.h"
#include"../include/global.h"

using namespace std;
using namespace vmath;

static bool hdrEnabled = false;
static HDR* hdr;
static sceneCamera *scenecamera;
static debugCamera *debugcamera;
static bool isDebugCameraOn = false;
static bool isAnimating = false;

#define SHOW_TEST_SCENE 		0
#define SHOW_MODEL_SCENE 		0
#define SHOW_CAMERA_SCENE 		1

mat4 programglobal::perspective;

void setupProgram(void) {
	try {
		// setupProgramTestEffect();
#if SHOW_CAMERA_SCENE
		setupProgramTestCamera();
#endif
#if SHOW_MODEL_SCENE
		setupProgramTestModel();
#endif
		hdr->setupProgram();
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void setupSceneCamera(void) {
	try {
		debugcamera = new debugCamera(vec3(0.0f, 0.0f, 5.0f), -90.0f, 0.0f);
		setupSceneCameraTestCamera(scenecamera);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void init(void) {
	try {
		//Object Creation
		hdr = new HDR(1.5f, 1.0f, 2048);

		//Inititalize
		// initTestEffect();
#if SHOW_CAMERA_SCENE
		initTestCamera();
#endif
#if SHOW_MODEL_SCENE
		initTestModel();
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
		if(hdrEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER, hdr->getFBO());
			glViewport(0, 0, hdr->getSize(), hdr->getSize());
		} else {
			glViewport(0, 0, window->getSize().width, window->getSize().height);
		}

		camera* currentCamera = isDebugCameraOn ? dynamic_cast<camera*>(debugcamera) : dynamic_cast<camera*>(scenecamera);

		glClearBufferfv(GL_COLOR, 0, vec4(0.5f, 1.0f, 0.2f, 1.0f));
		glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
		programglobal::perspective = perspective(45.0f, window->getSize().width / window->getSize().height, 0.1f, 1000.0f);
#if SHOW_CAMERA_SCENE
		renderTestCamera(currentCamera);
#endif
#if SHOW_MODEL_SCENE
		renderTestModel(dynamic_cast<camera*>(debugcamera));
#endif
		// renderTestEffect();

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
	scenecamera->updateT(0.001f);
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
	case XK_space:
		isAnimating = !isAnimating;
		break;
	}
	hdr->keyboardfunc(key);
	debugcamera->keyboardFunc(key);
}

void mouse(glwindow* window, int button, int action, int x, int y) {
	if(button == Button1) {
		debugcamera->mouseFunc(action, x, y);
	}
}

void uninit(void) {
	// uninitTestEffect();
#if SHOW_CAMERA_SCENE
	uninitTestCamera();
#endif
#if SHOW_MODEL_SCENE
	uninitTestModel();
#endif
	hdr->uninit();

	delete hdr;
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