#include <X11/Xlib.h>
#include <cstddef>
#include <iostream>

#include<GL/glew.h>
#include<GL/gl.h>

#include"../include/vmath.h"
#include"../include/glshaderloader.h"
#include"../include/testeffect.h"
#include "../include/hdr.h"
#include"../include/windowing.h"

using namespace std;
using namespace vmath;

static bool hdrEnabled = false;
static HDR* hdr;

void setupProgram(void) {
	setupProgramTestEffect();
	hdr->setupProgram();
}

void init(void) {
	hdr = new HDR(1.5f, 1.0f, 2048);
	
	initTestEffect();
	hdr->init();
}

void render(glwindow* window) {
	if(hdrEnabled) {
		glBindFramebuffer(GL_FRAMEBUFFER, hdr->getFBO());
		glViewport(0, 0, hdr->getSize(), hdr->getSize());
	} else {
		glViewport(0, 0, window->getSize().width, window->getSize().height);
	}

	glClearBufferfv(GL_COLOR, 0, vec4(0.5f, 1.0f, 0.2f, 1.0f));
	mat4 perspectiveMat = perspective(45.0f,(GLfloat) window->getSize().width / window->getSize().height, 0.01f, 100000.0f);
	renderTestEffect(perspectiveMat);
	
	if(hdrEnabled) {
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.1f, 0.1f, 1.0f));
		glViewport(0, 0, window->getSize().width, window->getSize().height);
		hdr->render();
	}
}

void keyboard(glwindow* window, int key) {
	switch(key) {
	case XK_Escape:
		window->close();
		break;
	case XK_space:
		hdrEnabled = !hdrEnabled;
		break;
	}
	hdr->keyboardfunc(key);
}

void uninit(void) {
	uninitTestEffect();
	hdr->uninit();

	delete hdr;
}

int main(int argc, char **argv) {
	glwindow* window = new glwindow("Our Planet", 0, 0, 1920, 1080, 460);
	init();
	setupProgram();
	window->setKeyboardFunc(keyboard);
	window->toggleFullscreen();
	while(!window->isClosed()) {
		window->processEvents();
		render(window);
		window->swapBuffers();
	}
	uninit();
	delete window;
	return 0;
}