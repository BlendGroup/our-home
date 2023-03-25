#include <X11/X.h>
#include<iostream>

#include<GL/glew.h>
#include<GL/gl.h>

#include"../include/vmath.h"
#include"../include/glshaderloader.h"
#include"../include/testeffect.h"
#include"../include/windowing.h"

using namespace std;
using namespace vmath;

void setupProgram(void) {
	setupProgramTestEffect();
}

void init(void) {
	initTestEffect();
}

void render(glwindow* window) {
	glViewport(0, 0, window->getWindowSize().width, window->getWindowSize().height);

	mat4 perspectiveMat = perspective(45.0f,(GLfloat) window->getWindowSize().width / window->getWindowSize().height, 0.01f, 100000.0f);

	renderTestEffect(perspectiveMat);
}

void uninit(void) {
	uninitTestEffect();
}

int main(int argc, char **argv) {
	glwindow* window = new glwindow();
	init();
	setupProgram();
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