#pragma once
#include<GL/glew.h>
#include<GL/gl.h>
#include<stdlib.h>
#include<math.h>

class sphere {
private:
	GLuint vao;
	GLuint vbo;
	GLuint eabo;
	GLuint numOfTriangles;
public:
	sphere(GLubyte stacks, GLubyte slices, GLfloat radius);
	void renderInstanced(GLsizei primcount);
	void render(void);
	~sphere();
};