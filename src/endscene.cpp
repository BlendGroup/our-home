#include<scenes/end.h>
#include<glshaderloader.h>
#include<global.h>
#include<vmath.h>
#include<iostream>
#include<eventmanager.h>
#include<crossfade.h>
#include<audio.h>
#include<gltextureloader.h>
#include<debugcamera.h>
#include<X11/keysym.h>

using namespace std;
using namespace vmath;

debugCamera* cam;

void endscene::setupProgram() {
}

void endscene::setupCamera() {
	cam = new debugCamera(vec3(0.0f), 0.0f, 0.0f);
}

void endscene::init() {
}

void endscene::render() {
}

void endscene::update(void) {
}

void endscene::reset(void) {
}

void endscene::uninit() {
}

void endscene::keyboardfunc(int key) {
}

camera* endscene::getCamera() {
	return cam;
}

void endscene::crossfade() {
}