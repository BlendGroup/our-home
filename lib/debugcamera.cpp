#include<debugcamera.h>
#include<windowing.h>
#include<X11/keysym.h>
#include<iostream>

using namespace std;
using namespace vmath;

#define calcFront(yaw, pitch) normalize(vec3(cos(radians(yaw)) * cos(radians(pitch)), sin(radians(pitch)), sin(radians(yaw)) * cos(radians(pitch)))) 

#define upVector vec3(0.0f, 1.0f, 0.0f)

debugCamera::debugCamera(vmath::vec3 pos, float yaw, float pitch) {
	this->pos = pos;
	this->yaw = yaw;
	this->pitch = pitch;
	this->front = calcFront(this->yaw, this->pitch);
}

void debugCamera::keyboardFunc(unsigned int key) {
	float speed = 0.1f;
	switch(key) {
	case XK_W: case XK_w:
		this->pos += speed * this->front;
		break;
	case XK_S: case XK_s:
		this->pos -= speed * this->front;
		break;
	case XK_D: case XK_d:
		this->pos += speed * normalize(cross(this->front, upVector));
		break;
	case XK_A: case XK_a:
		this->pos -= speed * normalize(cross(this->front, upVector));
		break;
	}
}

void debugCamera::mouseFunc(int action, int x, int y) {
	static float lastX;
	static float lastY;

	if(action == MOUSE_BUTTON_RELEASE) {
		lastX = 0.0f;
		lastY = 0.0f;
	} else if(action == MOUSE_BUTTON_PRESS) {
		lastX = x;
		lastY = y;
	} else if(action == MOUSE_BUTTON_MOVE) {
		float xoffset = x - lastX;
		float yoffset = lastY - y; 
		lastX = x;
		lastY = y;

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		this->yaw   += xoffset;
		this->pitch += yoffset;

		if(this->pitch > 89.0f) {
			this->pitch = 89.0f;
		} else if(this->pitch < -89.0f) {
			this->pitch = -89.0f;
		}
		this->front = calcFront(this->yaw, this->pitch);
	}
}

mat4 debugCamera::matrix() const {
	return lookat(this->pos, this->front + this->pos, upVector);
}

vec3 debugCamera::position() const {
	return this->pos;
}

vec3 debugCamera::getPosition(){
	return this->position;
}

debugCamera::~debugCamera() {

}