#ifndef __SHAPES__
#define __SHAPES__

#include<GL/glew.h>

struct buffer {
	GLuint vao;
	GLuint vbo;
	unsigned vertexCount;
	GLuint eabo;
};

class shaperenderer{
private:
	buffer quad;
	buffer sphere;
public:
	shaperenderer(void);
	void renderQuad(void);
	void renderSphere(void);
	~shaperenderer();
};

#endif