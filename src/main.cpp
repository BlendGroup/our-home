#include<iostream>
#include<memory>

#include<GL/glew.h>
#include<GL/gl.h>

#include<vmath.h>
#include<glshaderloader.h>
#include<scenecamera.h>
#include<scenecamerarig.h>
#include<debugcamera.h>
#include<hdr.h>
#include<windowing.h>
#include<errorlog.h>
#include<global.h>
#include<clhelper.h>
#include<gltextureloader.h>

#include<scenes/base.h>
#include<scenes/lab.h>

using namespace std;
using namespace vmath;

static bool hdrEnabled = true;
static HDR* hdr;
static sceneCamera* scenecamera;
static sceneCameraRig* scenecamerarig;
static debugCamera* debugcamera;
static bool isDebugCameraOn = true;
static bool isAnimating = false;
static basescene* currentScene;
static labscene* labScene;

mat4 programglobal::perspective;
clglcontext* programglobal::oclContext;
camera* programglobal::currentCamera;

void setupProgram(void) {
	try {
		programglobal::oclContext->compilePrograms({"shaders/terrain/calcnormals.cl"});
		hdr->setupProgram();
		labScene->setupProgram();
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void setupSceneCamera(void) {
	try {
		debugcamera = new debugCamera(vec3(0.0f, 1.0f, 5.0f), -90.0f, 0.0f);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void init(void) {
	try {
		//Object Creation
		hdr = new HDR(1.5f, 1.0f, 2048);
		programglobal::oclContext = new clglcontext(1);
		labScene = new labscene();

		//Inititalize
		initTextureLoader();
		hdr->init();
		labScene->init();

		currentScene = dynamic_cast<basescene*>(labScene);

		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void render(glwindow* window) {
	try {
		programglobal::currentCamera = isDebugCameraOn ? dynamic_cast<camera*>(debugcamera) : dynamic_cast<camera*>(scenecamera);

		if(hdrEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER, hdr->getFBO());
			glViewport(0, 0, hdr->getSize(), hdr->getSize());
		} else {
			glViewport(0, 0, window->getSize().width, window->getSize().height);
		}

		glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.3f, 0.2f, 1.0f));
		glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
		programglobal::perspective = perspective(45.0f, window->getSize().width / window->getSize().height, 0.1f, 1000.0f);

		currentScene->render();

		if(hdrEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.7f, 0.1f, 1.0f));
			glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
			glViewport(0, 0, window->getSize().width, window->getSize().height);
			hdr->render();
		}
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void update(void) {
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
}

void mouse(glwindow* window, int button, int action, int x, int y) {
	if(button == Button1) {
		debugcamera->mouseFunc(action, x, y);
	}
}

void uninit(void) {
	hdr->uninit();
	labScene->uninit();

	delete programglobal::oclContext;
	delete hdr;
	delete debugcamera;
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