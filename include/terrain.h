#ifndef __TERRAIN__
#define __TERRAIN__

#include<glbase.h>
#include<GL/glew.h>

#define MAX_PATCH_TESS_LEVEL 32
#define MIN_PATCH_TESS_LEVEL 2
#define MESH_SIZE 10

class terrain : public glbase {
private:
	GLuint heightMap;
	GLuint nomralMap;
public:
	terrain(GLuint heightMap);
	void setupProgram(void) override;
	void init(void) override;
	void render(void) override;
	void uninit(void) override;
	~terrain();
};

#endif