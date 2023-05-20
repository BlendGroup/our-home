#include<testterrain.h>
#include<terrain.h>
#include<vmath.h>
#include<gltextureloader.h>
#include<X11/keysym.h>
#include<global.h>

using namespace vmath;

static terrain* plainTerrain;

void setupProgramTestTerrain(void) {
	plainTerrain->setupProgram();
}

GLuint colortex;

void initTestTerrain(void) {
	// GLuint texture = createTexture2D("resources/textures/heightmap2.png", GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	// GLuint texture = programglobal::noiseGenerator->createNoiseTextureOnUniformInput(Noise2D, ivec2(TEXTURE_SIZE, TEXTURE_SIZE), ivec2(0, 0), 80.0f, 0.5f, 324);
	GLuint texture = programglobal::noiseGenerator->createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 0), 512.0f, 4, 3423);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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