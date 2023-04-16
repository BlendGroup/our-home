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
		this->renderHeightMap = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/render.frag"});
	} catch(string errorstring) {
		throwErr(errorstring);
	}
}

void terrain::init(void) {
	try {
		glGenTextures(1, &this->nomralMap);
		glBindTexture(GL_TEXTURE_2D, this->nomralMap);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB8, TEXTURE_SIZE, TEXTURE_SIZE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		this->normalMapCl = programglobal::oclContext->createGLCLTexture(CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, this->nomralMap);

		this->heightMapCl = programglobal::oclContext->createGLCLTexture(CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, this->heightMap);

		glCreateVertexArrays(1, &this->vao);
		glBindVertexArray(this->vao);

		glPatchParameteri(GL_PATCH_VERTICES, 4);

		this->normalKernel = programglobal::oclContext->getKernel("calcNormal");
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void terrain::render(void) {
}

void terrain::render(camera* cam) {
	this->renderHeightMap->use();
	glUniformMatrix4fv(this->renderHeightMap->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(this->renderHeightMap->getUniformLocation("vMat"), 1, GL_FALSE, cam->matrix());
	glUniformMatrix4fv(this->renderHeightMap->getUniformLocation("mMat"), 1, GL_FALSE, this->modelMatrix);
	glUniform1i(this->renderHeightMap->getUniformLocation("numMeshes"), MESH_SIZE);
	glUniform1f(this->renderHeightMap->getUniformLocation("maxTess"), MAX_PATCH_TESS_LEVEL);
	glUniform1f(this->renderHeightMap->getUniformLocation("minTess"), MIN_PATCH_TESS_LEVEL);
	glUniform1i(this->renderHeightMap->getUniformLocation("texHeight"), 0);
	glUniform1i(this->renderHeightMap->getUniformLocation("texColor"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->heightMap);
	glDrawArraysInstanced(GL_PATCHES, 0, 4, MESH_SIZE * MESH_SIZE);
}

void terrain::uninit(void) {
	// delete this->normalCalculator;
	delete this->renderHeightMap;
}