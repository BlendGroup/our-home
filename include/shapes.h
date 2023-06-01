#ifndef __SHAPES__
#define __SHAPES__

#include<GL/glew.h>

struct buffer {
	GLuint vao;
	GLuint vbo;
	unsigned vertexCount;
};

class shaperenderer{
private:
	buffer quad;
	buffer circle;
public:
	shaperenderer(void);
	void renderQuad(void);
	void renderCircle(void);
	~shaperenderer();
};

#endif