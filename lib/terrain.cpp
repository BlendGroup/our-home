#include<terrain.h>
#include<errorlog.h>
#include<iostream>
#include<global.h>
#include<vmath.h>

using namespace std;
using namespace vmath;

terrain::terrain(mat4 modelMatrix, GLuint heightMap) {
	this->heightMap.gl = heightMap;
	this->modelMatrix = modelMatrix;
}

void terrain::setupProgram(void) {
	try {
		// this->normalCalculator = new glshaderprogram({"shaders/terrain/calculatenormals.vert", "shaders/terrain/calculatenormals.frag"});
		this->renderHeightMap = new glshaderprogram({"shaders/terrain/render.vert", "shaders/terrain/render.tesc", "shaders/terrain/render.tese", "shaders/terrain/render.frag"});
		this->normalKernel = programglobal::oclContext->getKernel("calcNormal");

		// this->renderHeightMap->printUniforms(cout);

		programglobal::oclContext->setKernelParameters(this->normalKernel, {param(0, this->heightMap.cl), param(1, this->normalMap.cl)});
		size_t globalWorkSize[] = { 512, 512 };
		size_t localWorkSize[] = { 16, 16 };
		programglobal::oclContext->runCLKernel(this->normalKernel, 2, globalWorkSize, localWorkSize, {this->heightMap, this->normalMap});
		CLErr(clhelpererr = clFinish(programglobal::oclContext->getCommandQueue()));
	} catch(string errorstring) {
		throwErr(errorstring);
	}
}

void terrain::init(void) {
	try {
		this->heightMap = programglobal::oclContext->createCLfromGLTexture(CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, this->heightMap.gl);
		this->normalMap = programglobal::oclContext->createCLGLTexture(GL_TEXTURE_2D, GL_RGBA32F, TEXTURE_SIZE, TEXTURE_SIZE, CL_MEM_READ_WRITE);
		glBindTexture(GL_TEXTURE_2D, this->normalMap.gl);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);


		glCreateVertexArrays(1, &this->vao);
		glBindVertexArray(this->vao);

		glPatchParameteri(GL_PATCH_VERTICES, 4);
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
	glUniform3fv(this->renderHeightMap->getUniformLocation("cameraPos"), 1, cam->position());
	glUniform1i(this->renderHeightMap->getUniformLocation("texHeight"), 0);
	glUniform1i(this->renderHeightMap->getUniformLocation("texNormal"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->heightMap.gl);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->normalMap.gl);
	glDrawArraysInstanced(GL_PATCHES, 0, 4, MESH_SIZE * MESH_SIZE);
}

void terrain::uninit(void) {
	// delete this->normalCalculator;
	delete this->renderHeightMap;
}