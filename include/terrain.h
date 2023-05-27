#ifndef __TERRAIN__
#define __TERRAIN__

#include<glbase.h>
#include<glshaderloader.h>
#include<GL/glew.h>
#include<vmath.h>
#include<camera.h>
#include<clhelper.h>

class terrain {
private:
	GLuint vao;
	GLuint vbo;
	clglmem heightMap;
	clglmem normalMap;
	GLfloat minTess;
	GLfloat maxTess;
public:
	terrain(GLuint heightMap, bool calcNormal, GLfloat minTess, GLfloat maxTess);
	void render(void);
	inline GLuint getHeightMap() {
		return this->heightMap.gl;
	}
	inline GLuint getNormalMap() {
		return this->normalMap.gl;
	}
	inline GLfloat getMinTess(){
		return this->minTess;
	}
	inline GLfloat getMaxTess(){
		return this->maxTess;
	}
	~terrain();
};

#endif