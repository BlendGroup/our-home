#include<GL/glew.h>
#include<GL/gl.h>
#include<iostream>
#include"../include/vmath.h"
#include"../include/main.h"

using namespace std;
using namespace vmath;

winParam winSize;

void setupProgram(void) {
	
}

void init(void) {

}

void render(void) {
	glClearBufferfv(GL_COLOR, 0, vec4(0.5f, 1.0f, 0.2f, 1.0f));
}

void uninit(void) {

}
