#include<lake.h>

lake::lake() {
	glGenVertexArrays(1, &this->vao);
}

void lake::render() {
	glBindVertexArray(this->vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}