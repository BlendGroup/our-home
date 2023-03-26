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
	glClearBufferfv(GL_COLOR, 0, vec4(0.5f, 1.0f, 0.2f, 1.0f));
	glViewport(0, 0, window->getSize().width, window->getSize().height);

	mat4 perspectiveMat = perspective(45.0f,(GLfloat) window->getSize().width / window->getSize().height, 0.01f, 100000.0f);

	renderTestEffect(perspectiveMat);
}

void keyboard(glwindow* window, int key) {
	switch(key) {
	case XK_Escape:
		window->close();
		break;
	}
}

void uninit(void) {
	uninitTestEffect();
}

int main(int argc, char **argv) {
	glwindow* window = new glwindow("Our Planet", 0, 0, 1240, 1080, 460);
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