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
	GLuint getReflectionTexture(void);
	GLuint getRefractionTexture(void);
	GLuint getDepthTexture(void);
	void setReflectionFBO(void);
	void setRefractionFBO(void);
	void render(void);
};

#endif