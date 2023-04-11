#include<terrain.h>
#include<errorlog.h>
#include<iostream>
#include<global.h>
#include<vmath.h>

using namespace std;
using namespace vmath;

terrain::terrain(mat4 modelMatrix, GLuint heightMap) {
	this->heightMap = heightMap;
	this->modelMatrix = modelMatrix;
}

void terrain::setupProgram(void) {
	try {
		// this->normalCalculator = new glshaderprogram({"shaders/terrain/calculatenormals.vert", "shaders/terrain/calculatenormals.frag"});
		this->renderHeightMap = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.frag"});
	} catch(string errorstring) {
		throwErr(errorstring);
	}
}

void terrain::init(void) {
	glGenTextures(1, &this->nomralMap);
	glBindTexture(GL_TEXTURE_2D, this->nomralMap);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, TEXTURE_SIZE, TEXTURE_SIZE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void terrain::render(void) {
	this->renderHeightMap->use();
	glUniformMatrix4fv(this->renderHeightMap->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(this->renderHeightMap->getUniformLocation("vMat"), 1, GL_FALSE, mat4::identity());
	glUniformMatrix4fv(this->renderHeightMap->getUniformLocation("mMat"), 1, GL_FALSE, this->modelMatrix);
}

void terrain::uninit(void) {
	// delete this->normalCalculator;
	delete this->renderHeightMap;
}