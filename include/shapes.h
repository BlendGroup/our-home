#ifndef __SHAPES__
#define __SHAPES__

#include<GL/glew.h>

struct quad {
	GLuint vao;
	GLuint vbo;
};

class shaperenderer{
private:
	quad q;
public:
	shaperenderer(void);
	void renderQuad(void);
	~shaperenderer();
};

#endif