#include<shapes.h>
#include<vmath.h>

using namespace vmath;

void initQuad(quad &q) {
	GLfloat array[] = {
		//Position			//Normal			//TexCoords
		1.0f, 1.0f, 0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 1.0f,
		-1.0f, 1.0f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 1.0f,
		1.0f, -1.0f, 0.0f,	0.0f, 0.0f, 1.0f,	1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,	0.0f, 0.0f, 1.0f,	0.0f, 0.0f
	};
	glGenVertexArrays(1, &q.vao);
	glGenBuffers(1, &q.vbo);
	glBindVertexArray(q.vao);
	glBindBuffer(GL_ARRAY_BUFFER, q.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(array), array, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

shaperenderer::shaperenderer(void) {
	initQuad(this->q);
}

void shaperenderer::renderQuad(void) {
	glBindVertexArray(this->q.vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

shaperenderer::~shaperenderer() {
	glDeleteVertexArrays(1, &this->q.vao);
	glDeleteBuffers(1, &this->q.vbo);
}