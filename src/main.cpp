#include<iostream>

#include<GL/glew.h>
#include<GL/gl.h>

#include"../include/vmath.h"
#include"../include/glshaderloader.h"
#include"../include/testeffect.h"
#include"../include/testcamera.h"
#include"../include/testmodel.h"
#include"../include/hdr.h"
#include"../include/windowing.h"
#include"../include/errorlog.h"
#include"../include/global.h"

using namespace std;
using namespace vmath;

static bool hdrEnabled = false;
static HDR* hdr;

mat4 programglobal::perspective;

void setupProgram(void) {
	try {
		// setupProgramTestEffect();
		setupProgramTestCamera();
		// setupProgramTestModel();
		hdr->setupProgram();
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
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		// initTestEffect();
		initTestCamera();
		// initTestModel();
		hdr->init();

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

		glClearBufferfv(GL_COLOR, 0, vec4(0.5f, 1.0f, 0.2f, 1.0f));
		glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));
		programglobal::perspective = perspective(45.0f, window->getSize().width / window->getSize().width, 0.1f, 1000.0f);
		renderTestCamera();
		// renderTestModel();
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

void keyboard(glwindow* window, int key) {
	switch(key) {
	case XK_Escape:
		window->close();
		break;
	case XK_F1:
		window->toggleFullscreen();
		break;
	case XK_space:
		hdrEnabled = !hdrEnabled;
		break;
	}
	hdr->keyboardfunc(key);
	keyboardFuncTestCamera(key);
}

void mouse(glwindow* window, int button, int action, int x, int y) {
	if(button == Button1) {
		mouseFuncTestCamera(action, x, y);
	}
}

void uninit(void) {
	// uninitTestEffect();
	uninitTestCamera();
	// uninitTestModel();
	hdr->uninit();

	delete hdr;
}

int main(int argc, char **argv) {
	try {
		glwindow* window = new glwindow("Our Planet", 0, 0, 1920, 1080, 460);
		init();
		setupProgram();
		window->setKeyboardFunc(keyboard);
		window->setMouseFunc(mouse);
		window->setFullscreen(true);
		while(!window->isClosed()) {
			window->processEvents();
			render(window);
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