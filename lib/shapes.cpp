#include<shapes.h>
#include<vmath.h>

using namespace vmath;

void initQuad(buffer &q) {
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

	q.vertexCount = 4;
}

void initCircle(buffer &q, unsigned slices) {
	GLfloat array[(slices + 2) * 8];
	int k = 0;
	for(int i = 0; i <= slices; i++) {
		GLfloat theta = 2.0f * M_PI * ((float)i / slices);
		array[k++] = sin(theta);array[k++] = cos(theta);array[k++] = 0.0f;
		
		array[k++] = 0.0f;array[k++] = 0.0f;array[k++] = 1.0f;
	
		array[k++] = sin(theta) * 0.5f + 0.5f;
		array[k++] = cos(theta) * 0.5f + 0.5f;
	}
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

	q.vertexCount = slices + 2;
}

shaperenderer::shaperenderer(void) {
	initQuad(this->quad);
	initCircle(this->circle, 50);
}

void shaperenderer::renderQuad(void) {
	glBindVertexArray(this->quad.vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, this->quad.vertexCount);
	glBindVertexArray(0);
}

void shaperenderer::renderCircle(void) {
	glBindVertexArray(this->circle.vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, this->circle.vertexCount);
	glBindVertexArray(0);
}

shaperenderer::~shaperenderer() {
	glDeleteVertexArrays(1, &this->quad.vao);
	glDeleteBuffers(1, &this->quad.vbo);
}