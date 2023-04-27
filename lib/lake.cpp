#include<lake.h>
#include<GL/glew.h>
#include<GL/gl.h>

lake::lake() {

}

void lake::setupProgram(void) {

}

void lake::init(void) {
	glGenVertexArrays(1, &this->vao);
	glBindVertexArray(this->vao);

	
}

void lake::render(void) {

}

void lake::uninit(void) {

}

lake::~lake() {
	
}