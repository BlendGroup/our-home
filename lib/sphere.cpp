#include<sphere.h>

#define vertex_stride 8 * 4
#define position_offset (0)
#define normal_offset (3 * 4)
#define texcoord_offset (6 * 4)

sphere::sphere(GLubyte stacks, GLubyte slices, GLfloat radius) {
	GLfloat* vertices = (GLfloat*)calloc(slices * stacks * 8, sizeof(GLfloat));
	for(int i = 0; i < stacks; i++) {
		GLfloat phi = M_PI * ((GLfloat)i / (stacks - 1));
		for(int j = 0; j < slices; j++) {
			GLfloat theta = 2.0f * M_PI * ((GLfloat)j / (slices - 1));
			//Position
			vertices[(i * slices + j) * 8 + 0] = sin(phi) * sin(theta) * radius;
			vertices[(i * slices + j) * 8 + 1] = cos(phi) * radius;
			vertices[(i * slices + j) * 8 + 2] = sin(phi) * cos(theta) * radius;
			//Normal
			vertices[(i * slices + j) * 8 + 3] = sin(phi) * sin(theta);
			vertices[(i * slices + j) * 8 + 4] = cos(phi);
			vertices[(i * slices + j) * 8 + 5] = sin(phi) * cos(theta);
			//Texcoord
			vertices[(i * slices + j) * 8 + 6] = ((GLfloat)j / (slices - 1));
			vertices[(i * slices + j) * 8 + 7] = 1.0f - ((GLfloat)i / (stacks - 1));
		}
	}

	GLuint* elements = (GLuint*)calloc((slices - 1) * (stacks - 1) * 6, sizeof(GLuint));
	this->numOfTriangles = (slices - 1) * (stacks - 1) * 6;
	for(int i = 0; i < stacks - 1; i++) {
		for(int j = 0; j < slices - 1; j++) {
			elements[(i * (slices - 1) + j) * 6 + 0] = i * slices + j;
			elements[(i * (slices - 1) + j) * 6 + 1] = (i + 1) * slices + j;
			elements[(i * (slices - 1) + j) * 6 + 2] = (i + 1) * slices + j + 1;

			elements[(i * (slices - 1) + j) * 6 + 3] = (i + 1) * slices + j + 1;
			elements[(i * (slices - 1) + j) * 6 + 4] = i * slices + j + 1;
			elements[(i * (slices - 1) + j) * 6 + 5] = i * slices + j;
		}
	}
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, slices * stacks * 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)position_offset);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)normal_offset);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertex_stride, (GLvoid*)texcoord_offset);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &this->eabo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->eabo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (slices - 1) * (stacks - 1) * 6 * sizeof(GLuint), elements, GL_STATIC_DRAW);
}

void sphere::render(GLsizei primcount) {
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->eabo);
	glDrawElementsInstanced(GL_TRIANGLES, this->numOfTriangles, GL_UNSIGNED_INT, 0, primcount);
}

sphere::~sphere() {
	glDeleteVertexArrays(1, &this->vao);
	glDeleteBuffers(1, &this->vbo);
	glDeleteBuffers(1, &this->eabo);
}