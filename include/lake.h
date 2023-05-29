#ifndef __LAKE__
#define __LAKE__

#include<GL/glew.h>
#include<GL/gl.h>

class lake {
private:
	GLuint vaoEmpty;
	GLuint fboReflection;
	GLuint texColorReflection;
	GLuint rboDepthReflection;
	GLuint fboRefraction;
	GLuint texColorRefraction;
	GLuint texDepthRefraction;
public:
	lake();
	void render();
};

#endif