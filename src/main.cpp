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
#include<godrays.h>
#include<crossfade.h>

#include<scenes/base.h>
#include<scenes/title.h>
#include<scenes/lab.h>
#include<scenes/day.h>
#include<scenes/night.h>

using namespace std;
using namespace vmath;

static bool hdrEnabled = true;
static debugCamera* debugcamera;
static bool isDebugCameraOn = false;
static vector<basescene*> sceneList;
static basescene* currentScene;
static glwindow* window;

vmath::mat4 programglobal::perspective;
camera* programglobal::currentCamera;
double programglobal::deltaTime;
debugMode_t programglobal::debugMode = NONE;
clglcontext* programglobal::oclContext;
shaperenderer* programglobal::shapeRenderer;
bool programglobal::isAnimating = false;
HDR* programglobal::hdr;

void setupProgram(void) {
	try {
		programglobal::hdr->setupProgram();
		for(basescene* b : sceneList) {
			b->setupProgram();
		}
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void setupSceneCamera(void) {
	try {
		// debugcamera = new debugCamera(vec3(-37.869f, 7.20367f, -50.1925f), -12.6, -104.6);
		// debugcamera = new debugCamera(vec3(0.0f, 0.0f, 1.0f), -90.0f, 0.0f);
		debugcamera = new debugCamera(vec3(64.3364f, -0.71273f, -77.4934f), -90.0f, 0.0f);
		for(basescene* b : sceneList) {
			b->setupCamera();
		}
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void init(void) {
	try {
		//Object Creation
		programglobal::hdr = new HDR(1.5f, 1.0f, 2048);
		programglobal::oclContext = new clglcontext(1);
		programglobal::oclContext->compilePrograms({"shaders/opensimplexnoise.cl"});
		programglobal::shapeRenderer = new shaperenderer();
		crossfader::init();
		sceneList.insert(sceneList.begin(), {
			new titlescene(),
			new labscene(),
			// new dayscene(),
			// new nightscene()
		});

		//Inititalize
		alutInit(0, NULL);
		initTextureLoader();
		programglobal::hdr->init();
		programglobal::hdr->setBloom(true);
		for(basescene* b : sceneList) {
			b->init();
		} 

		playNextScene();
		// playNextScene();
		// playNextScene();
		// playNextScene();

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
			glBindFramebuffer(GL_FRAMEBUFFER, programglobal::hdr->getFBO());
			glViewport(0, 0, programglobal::hdr->getSize(), programglobal::hdr->getSize());
			glClearBufferfv(GL_COLOR, 1, vec4(0.0f, 0.0f, 0.0f, 1.0f));
			glClearBufferfv(GL_COLOR, 2, vec4(0.0f, 0.0f, 0.0f, 1.0f));
		} else {
			glViewport(0, 0, window->getSize().width, window->getSize().height);
		}

		glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
		glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
		programglobal::perspective = perspective(45.0f, window->getSize().width / window->getSize().height, 0.1f, 1000.0f);

		currentScene->render();

		if(hdrEnabled) {
			glBindFramebuffer(GL_FRAMEBUFFER,0);
			glClearBufferfv(GL_COLOR, 0, vec4(0.0f, 0.0f, 0.0f, 1.0f));
			glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
			glViewport(0, 0, window->getSize().width, window->getSize().height);
			programglobal::hdr->render();
			currentScene->crossfade();
		}
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void resetFBO() {
	if(hdrEnabled) {
		glBindFramebuffer(GL_FRAMEBUFFER, programglobal::hdr->getFBO());
		glViewport(0, 0, programglobal::hdr->getSize(), programglobal::hdr->getSize());
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, window->getSize().width, window->getSize().height);
	}
}

void update(void) {
	currentScene->update();
}

void resetScene(void) {
	currentScene->reset();
}

void keyboard(glwindow* window, int key) {
	switch(key) {
	case XK_Tab:
		cout<<debugcamera<<endl;
		break;
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
		programglobal::debugMode = NONE;
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
		programglobal::isAnimating = !programglobal::isAnimating;
		break;
	}
	debugcamera->keyboardFunc(key);
	currentScene->keyboardfunc(key);
}

void playNextScene(void) {
	static int current = -1;
	current++;
	if(current < sceneList.size()) {
		currentScene = sceneList[current];
	}
}

void mouse(glwindow* window, int button, int action, int x, int y) {
	if(button == Button1) {
		debugcamera->mouseFunc(action, x, y);
	}
}

void uninit(void) {
	programglobal::hdr->uninit();
	for(basescene* b : sceneList) {
		b->uninit();
		delete b;
	}
	crossfader::uninit();
	delete programglobal::oclContext;
	delete programglobal::hdr;
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
	try {
		window = new glwindow("Our Planet", 0, 0, 1920, 1080, 460);
		auto initstart = chrono::steady_clock::now();
		window->setKeyboardFunc(keyboard);
		window->setMouseFunc(mouse);
		init();
		window->setFullscreen(true);
		auto initend = chrono::steady_clock::now();
		chrono::duration<double> diff = initend - initstart;
		cout<<"Time taken to initialize "<<diff.count()<<" sec"<<endl;
		setupProgram();
		setupSceneCamera();
		while(!window->isClosed()) {
			window->processEvents();
			render(window);
			programglobal::deltaTime = getDeltaTime(window);
			update();
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