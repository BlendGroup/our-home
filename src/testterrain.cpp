#include<testterrain.h>
#include<terrain.h>
#include<vmath.h>

using namespace vmath;

static terrain* plainTerrain;

void setupProgramTestTerrain(void) {
	plainTerrain->setupProgram();
}

void initTestTerrain(void) {
	plainTerrain = new terrain(translate(0.0f, 0.0f, -4.0f), 0);

	plainTerrain->init();
}

void renderTestTerrain(void) {
	plainTerrain->render();
}

void uninitTestTerrain(void) {
	plainTerrain->uninit();
}