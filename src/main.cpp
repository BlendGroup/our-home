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
	glClearBufferfv(GL_COLOR, 0, vec4(0.5f, 1.0f, 0.2f, 1.0f));
	glViewport(0, 0, window->getWindowSize().width, window->getWindowSize().height);

	renderTestEffect();
}

void keyboard(glwindow* window, int key) {
	switch(key) {
	case XK_Escape:
		window->closeWindow();
		break;
	}
}

void uninit(void) {
	uninitTestEffect();
}

int main(int argc, char **argv) {
	glwindow* window = new glwindow("Our Planet", 460);
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