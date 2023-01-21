#include<iostream>

#include"../include/vmath.h"
#include"../include/commongl.h"
#include"../include/glshaderloader.h"

#include"../include/testeffect.h"

#include"../include/main.h"

using namespace std;
using namespace vmath;

winParam winSize;

void setupProgram(void) {
	setupProgramTestEffect();
}

void init(void) {
	initTestEffect();
}

void render(void) {
	glClearBufferfv(GL_COLOR, 0, vec4(0.5f, 1.0f, 0.2f, 1.0f));
	glViewport(0, 0, winSize.w, winSize.h);

	renderTestEffect();
}

void uninit(void) {
	uninitTestEffect();
}
