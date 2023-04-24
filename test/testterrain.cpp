#include<testterrain.h>
#include<terrain.h>
#include<vmath.h>
#include<gltextureloader.h>
#include<X11/keysym.h>

using namespace vmath;

static terrain* plainTerrain;

void setupProgramTestTerrain(void) {
	plainTerrain->setupProgram();
}

GLuint colortex;

void initTestTerrain(void) {
	GLuint texture = createTexture2D("resources/textures/heightmap2.png", GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

	plainTerrain = new terrain(translate(0.0f, -1.0f, -2.0f), texture);
	
	plainTerrain->init();

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void renderTestTerrain() {
	plainTerrain->render();
}

void keyboardFuncTestTerrain(int key) {
	switch(key) {
	case XK_T: case XK_t:
		// plainTerrain->absToogle = !plainTerrain->absToogle;
		break;
	}
}

void uninitTestTerrain(void) {
	plainTerrain->uninit();
}