#include<AL/alut.h>
#include<iostream>
#include<memory>
#include<chrono>

#include<GL/glew.h>
#include<GL/gl.h>

#include<vmath.h>
#include<scenecamera.h>
#include<debugcamera.h>
#include<hdr.h>
#include<windowing.h>
#include<errorlog.h>
#include<global.h>
#include<clhelper.h>
#include<gltextureloader.h>
#include<shapes.h>

#include<scenes/base.h>
#include<scenes/lab.h>
#include<scenes/day.h>
#include<scenes/title.h>

using namespace std;
using namespace vmath;

static bool hdrEnabled = true;
static HDR* hdr;
static debugCamera* debugcamera;
static bool isDebugCameraOn = true;
static bool isAnimating = false;
static basescene* currentScene;
static titlescene* titleScene;
static labscene* labScene;
static dayscene* dayScene;

vmath::mat4 programglobal::perspective;
camera* programglobal::currentCamera;
double programglobal::deltaTime;
debugMode_t programglobal::debugMode;
clglcontext* programglobal::oclContext;
shaperenderer* programglobal::shapeRenderer;

void setupProgram(void) {
	try {
		hdr->setupProgram();
		titleScene->setupProgram();
		labScene->setupProgram();
		dayScene->setupProgram();
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void setupSceneCamera(void) {
	try {
		debugcamera = new debugCamera(vec3(0.0f, 0.0f, 5.0f), -90.0f, 0.0f);
		titleScene->setupCamera();
		labScene->setupCamera();
		dayScene->setupCamera();
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void init(void) {
	try {
		//Object Creation
		hdr = new HDR(1.5f, 1.0f, 2048);
		programglobal::oclContext = new clglcontext(1);
		programglobal::oclContext->compilePrograms({"shaders/terrain/calcnormals.cl", "shaders/opensimplexnoise.cl"});
		programglobal::shapeRenderer = new shaperenderer();
		titleScene = new titlescene();
		labScene = new labscene();
		dayScene = new dayscene();

		//Inititalize
		alutInit(0, NULL);
		initTextureLoader();
		hdr->init();
		hdr->toggleBloom(true);
		titleScene->init();
		labScene->init();
		dayScene->init();

		currentScene = dynamic_cast<basescene*>(titleScene);
		// currentScene = dynamic_cast<basescene*>(labScene);
		// currentScene = dynamic_cast<basescene*>(dayScene);

		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void render(glwindow* window) {
	try {
		programglobal::currentCamera = isDebugCameraOn ? dynamic_cast<camera*>(debugcamera) : dynamic_cast<camera*>(currentScene->getCamera());

		if(hdrEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER, hdr->getFBO());
			glViewport(0, 0, hdr->getSize(), hdr->getSize());
			glClearBufferfv(GL_COLOR, 1, vec4(0.0f, 0.0f, 0.0f, 1.0f));
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
	if(isAnimating) {
		currentScene->update();
	}
}

void resetScene(void) {
	currentScene->reset();
}

std::chrono::time_point<typename chrono::steady_clock> start;

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
	case XK_F4:
		resetScene();
		break;
	case XK_F5:
		programglobal::debugMode = CAMERA;
		break;
	case XK_F6:
		programglobal::debugMode = MODEL;
		break;
	case XK_F7:
		programglobal::debugMode = SPLINE;
		break;
	case XK_F8:
		programglobal::debugMode = LIGHT;
		break;
	case XK_space:
		isAnimating = !isAnimating;
		start = chrono::steady_clock::now();
		break;
	}
	debugcamera->keyboardFunc(key);
	currentScene->keyboardfunc(key);
}

void callMeToExit() {
	auto end = chrono::steady_clock::now();
	chrono::duration<double> diff = end - start;
	cout<<"Time taken to render "<<diff.count()<<" sec"<<endl;
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

double getDeltaTime(glwindow *win) {
	static double prev = win->getTime();
	double current = win->getTime();
	double delta = current - prev;
	prev = current;
	return delta;
}

int main(int argc, char **argv) {
#define SPEED_MULTIPLIER 1
	try {
		glwindow* window = new glwindow("Our Planet", 0, 0, 1920, 1080, 460);
		auto initstart = chrono::steady_clock::now();
		init();
		auto initend = chrono::steady_clock::now();
		chrono::duration<double> diff = initend - initstart;
		cout<<"Time taken to initialize "<<diff.count()<<" sec"<<endl;
		setupProgram();
		setupSceneCamera();
		window->setKeyboardFunc(keyboard);
		window->setMouseFunc(mouse);
		window->setFullscreen(true);
		while(!window->isClosed()) {
			window->processEvents();
			render(window);
			programglobal::deltaTime = getDeltaTime(window);
			#ifdef DEBUG
			programglobal::deltaTime *= SPEED_MULTIPLIER;
			#endif
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