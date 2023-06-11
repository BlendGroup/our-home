#ifndef __SHAPES__
#define __SHAPES__

#include<GL/glew.h>

struct buffer {
	GLuint vao;
	GLuint vbo;
	GLuint eabo;
	unsigned vertexCount;
};

class shaperenderer{
private:
	buffer quad;
public:
	shaperenderer(void);
	void renderQuad(void);
	~shaperenderer();
};

#endif