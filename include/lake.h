#ifndef __LAKE__
#define __LAKE__

#include<GL/glew.h>
#include<GL/gl.h>

class lake {
private:
	GLuint vao;
public:
	lake();
	void render();
};

#endif