#ifndef __TERRAIN__
#define __TERRAIN__

#include<glbase.h>
#include<glshaderloader.h>
#include<GL/glew.h>
#include<vmath.h>
#include<camera.h>
#include<clhelper.h>

#define TEXTURE_SIZE 512
#define MAX_PATCH_TESS_LEVEL 32
#define MIN_PATCH_TESS_LEVEL 5
#define MESH_SIZE 64

class terrain {
private:
	GLuint vao;
	clglmem heightMap;
	clglmem normalMap;
public:
	terrain(GLuint heightMap);
	void render(void);
	inline GLuint getHeightMap() {
		return this->heightMap.gl;
	}
	inline GLuint getNormalMap() {
		return this->normalMap.gl;
	}
	~terrain();
};

#endif