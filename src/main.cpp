#include <AL/alut.h>
#include <cstdlib>
#include<iostream>
#include<memory>
#include<chrono>

#include<GL/glew.h>
#include<GL/gl.h>

#include<vmath.h>
#include<glshaderloader.h>
#include<scenecamera.h>
#define CAMERA_RIG_SCALER 0.01
#include<scenecamerarig.h>
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
static vector<sceneCamera*> scenecamera;
static sceneCameraRig* scenecamerarig;
static debugCamera* debugcamera;
static sceneCamera* currentSceneCamera;
static bool isDebugCameraOn = true;
static bool isAnimating = false;
static bool isSceneCameraEditing = false;
static basescene* currentScene;
static titlescene* titleScene;
static labscene* labScene;
static dayscene* dayScene;

vmath::mat4 programglobal::perspective;
camera* programglobal::currentCamera;
double programglobal::deltaTime;

clglcontext* programglobal::oclContext;
shaperenderer* programglobal::shapeRenderer;

void setupProgram(void) {
	try {
		hdr->setupProgram();
		// titleScene->setupProgram();
		labScene->setupProgram();
		// dayScene->setupProgram();
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void setupSceneCamera(void) {
	try {
		debugcamera = new debugCamera(vec3(0.0f, 0.0f, 5.0f), -90.0f, 0.0f);
		// scenecamera.push_back(titleScene->setupCamera());
		scenecamera.push_back(labScene->setupCamera());
		// scenecamera.push_back(dayScene->setupCamera());
		
#ifdef DEBUG
		scenecamerarig = new sceneCameraRig(scenecamera[0]);
		scenecamerarig->setRenderPath(true);
		scenecamerarig->setRenderPathPoints(true);
		scenecamerarig->setRenderFront(true);
		scenecamerarig->setRenderFrontPoints(true);
		scenecamerarig->setRenderPathToFront(true);
		scenecamerarig->setScalingFactor(0.01f);
#endif
		currentSceneCamera = scenecamera[0];
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
		// titleScene->init();
		labScene->init();
		// dayScene->init();

		// currentScene = dynamic_cast<basescene*>(titleScene);
		currentScene = dynamic_cast<basescene*>(labScene);
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
		void checkIfDone(double);
		// checkIfDone(33.0f);
		programglobal::currentCamera = isDebugCameraOn ? dynamic_cast<camera*>(debugcamera) : dynamic_cast<camera*>(currentSceneCamera);

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

		scenecamerarig->render();

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
#define CAMERA_SPEED 0.025f

	if(isAnimating) {
		currentSceneCamera->updateT(programglobal::deltaTime * CAMERA_SPEED);
	}
	currentScene->update(currentSceneCamera);
}

void resetCamera(void) {
		currentSceneCamera->resetT();
}

std::chrono::time_point<typename chrono::steady_clock> start;

void keyboard(glwindow* window, int key) {
	switch(key) {
	case XK_Escape:
		// cout<<currentSceneCamera<<endl;
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
	case XK_F4:
		resetCamera();
		break;
	case XK_F5:
		isSceneCameraEditing = !isSceneCameraEditing;
		break;
	case XK_space:
		isAnimating = !isAnimating;
		start = chrono::steady_clock::now();
		break;
	case XK_Tab:
		cout<<currentSceneCamera<<endl;
		break;
	case XK_Left:
		currentSceneCamera->updateT(-0.001f);
		break;
	case XK_Right:
		currentSceneCamera->updateT(0.001f);
		break;
	}
	hdr->keyboardfunc(key);
	debugcamera->keyboardFunc(key);
#ifdef DEBUG
	if(isSceneCameraEditing) {
		scenecamerarig->keyboardfunc(key);
	} else {
		currentScene->keyboardfunc(key);
	}
#endif
}

void checkIfDone(double c) {
	auto end = chrono::steady_clock::now();
	chrono::duration<double> diff = end - start;
	if(diff.count() >= c && isAnimating) {
		cout<<"Camera Spline Pos:"<<currentSceneCamera->getDistanceOnSpline()<<endl;
		exit(0);
	}
}

void callMeToExit() {
	auto end = chrono::steady_clock::now();
	chrono::duration<double> diff = end - start;
	cout<<"Time taken to render "<<diff.count()<<" sec"<<endl;
	exit(0);
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