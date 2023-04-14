#include<testterrain.h>
#include<terrain.h>
#include<vmath.h>

using namespace vmath;

static terrain* plainTerrain;

void setupProgramTestTerrain(void) {
	plainTerrain->setupProgram();
}

void initTestTerrain(void) {
	plainTerrain = new terrain(translate(0.0f, -1.0f, -2.0f), 0);

	plainTerrain->init();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void renderTestTerrain(camera* cam) {
	plainTerrain->render(cam);
}

void uninitTestTerrain(void) {
	plainTerrain->uninit();
}