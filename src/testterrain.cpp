#include<testterrain.h>
#include<terrain.h>
#include<vmath.h>
#include<gltextureloader.h>

using namespace vmath;

static terrain* plainTerrain;

void setupProgramTestTerrain(void) {
	plainTerrain->setupProgram();
}

GLuint colortex;

void initTestTerrain(void) {
	GLuint texture = createTexture2D("resources/textures/heightmap2.png", GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	colortex = createTexture2D("resources/textures/diffusemap.png", GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	plainTerrain = new terrain(translate(0.0f, -1.0f, -2.0f), texture);
	
	plainTerrain->init();

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void renderTestTerrain(camera* cam) {
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, colortex);
	plainTerrain->render(cam);
}

void uninitTestTerrain(void) {
	plainTerrain->uninit();
}