#ifndef __TERRAIN__
#define __TERRAIN__

#include<glbase.h>
#include<glshaderloader.h>
#include<GL/glew.h>
#include<vmath.h>
#include<camera.h>

#define TEXTURE_SIZE 1024
#define MAX_PATCH_TESS_LEVEL 32
#define MIN_PATCH_TESS_LEVEL 5
#define MESH_SIZE 20

class terrain : public glbase {
private:
	GLuint heightMap;
	GLuint nomralMap;
	GLuint vao;
	GLuint vbo;
	glshaderprogram* normalCalculator;
	glshaderprogram* renderHeightMap;
	vmath::mat4 modelMatrix;
public:
	terrain(vmath::mat4 modelMatrix, GLuint heightMap);
	void setupProgram(void) override;
	void init(void) override;
	void render(void) override;
	void render(camera* cam);
	void uninit(void) override;
};

#endif