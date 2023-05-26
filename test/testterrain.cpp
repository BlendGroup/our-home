#include<testterrain.h>
#include<terrain.h>
#include<vmath.h>
#include<gltextureloader.h>
#include<X11/keysym.h>
#include<global.h>
#include<opensimplexnoise.h>

using namespace vmath;

static terrain* plainTerrain;
static glshaderprogram* terrainRenderer;

void setupProgramTestTerrain(void) {
	terrainRenderer = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/render.frag"});
}

GLuint colortex;

void initTestTerrain(void) {
	// GLuint texture = createTexture2D("resources/textures/heightmap2.png", GL_LINEAR, GL_LINEAR, GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	// GLuint texture = programglobal::noiseGenerator->createNoiseTextureOnUniformInput(Noise2D, ivec2(TEXTURE_SIZE, TEXTURE_SIZE), ivec2(0, 0), 80.0f, 0.5f, 324);
	// GLuint texture = programglobal::noiseGenerator->createFBMTexture2D(ivec2(1024, 1024), ivec2(0, 0), 800.0f, 3, 3423);
	GLuint texture = opensimplexnoise::createTurbulenceFBMTexture2D(ivec2(1024, 1024), ivec2(0, 0), 1200.0f, 6, 0.8f, 3423);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	plainTerrain = new terrain(texture);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void renderTestTerrain() {
	terrainRenderer->use();
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(terrainRenderer->getUniformLocation("mMat"), 1, GL_FALSE, translate(0.0f, 0.0f, -30.0f));
	glUniform1i(terrainRenderer->getUniformLocation("numMeshes"), MESH_SIZE);
	glUniform1f(terrainRenderer->getUniformLocation("maxTess"), MAX_PATCH_TESS_LEVEL);
	glUniform1f(terrainRenderer->getUniformLocation("minTess"), MIN_PATCH_TESS_LEVEL);
	glUniform3fv(terrainRenderer->getUniformLocation("cameraPos"), 1, programglobal::currentCamera->position());
	glUniform1i(terrainRenderer->getUniformLocation("texHeightValley"), 0);
	glUniform1i(terrainRenderer->getUniformLocation("texNormalValley"), 1);
	glUniform1i(terrainRenderer->getUniformLocation("texHeightMountain"), 2);
	glUniform1i(terrainRenderer->getUniformLocation("texNormalMountain"), 3);
	glUniform1f(terrainRenderer->getUniformLocation("amplitudeValley"), 5.0f);
	glUniform1f(terrainRenderer->getUniformLocation("amplitudeMountain"), 60.0f);
	glBindTextureUnit(0, plainTerrain->getHeightMap());
	glBindTextureUnit(1, plainTerrain->getNormalMap());
	glBindTextureUnit(2, plainTerrain->getHeightMap());
	glBindTextureUnit(3, plainTerrain->getNormalMap());
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
	delete plainTerrain;
}