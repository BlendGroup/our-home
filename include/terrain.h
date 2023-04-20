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

class terrain : public glbase {
private:
	GLuint vao;
	clglmem heightMap;
	clglmem normalMap;

	glshaderprogram* renderHeightMap;
	cl_kernel normalKernel;

	vmath::mat4 modelMatrix;
public:
	terrain(vmath::mat4 modelMatrix, GLuint heightMap);
	void setupProgram(void) override;
	void init(void) override;
	void render(void) override;
	void uninit(void) override;
	~terrain();
};

#endif