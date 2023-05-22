#include<terrain.h>
#include<errorlog.h>
#include<iostream>
#include<global.h>
#include<vmath.h>

using namespace std;
using namespace vmath;

terrain::terrain(GLuint heightMap) {
	try {
		this->heightMap = programglobal::oclContext->createCLfromGLTexture(CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, heightMap);
		this->normalMap = programglobal::oclContext->createCLGLTexture(GL_TEXTURE_2D, GL_RGBA32F, TEXTURE_SIZE, TEXTURE_SIZE, CL_MEM_READ_WRITE);
		glBindTexture(GL_TEXTURE_2D, this->normalMap.gl);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		cl_kernel normalKernel = programglobal::oclContext->getKernel("calcNormal");

		programglobal::oclContext->setKernelParameters(normalKernel, {param(0, this->heightMap.cl), param(1, this->normalMap.cl)});
		size_t globalWorkSize[] = { TEXTURE_SIZE, TEXTURE_SIZE };
		size_t localWorkSize[] = { 16, 16 };
		programglobal::oclContext->runCLKernel(normalKernel, 2, globalWorkSize, localWorkSize, {this->heightMap, this->normalMap});
		CLErr(clhelpererr = clFinish(programglobal::oclContext->getCommandQueue()));

		vec3 *data = new vec3[MESH_SIZE * MESH_SIZE];

		for(int i = 0; i < MESH_SIZE; i++) {
			for(int j = 0; j < MESH_SIZE; j++) {
				
			}
		}

		glCreateVertexArrays(1, &this->vao);
		glBindVertexArray(this->vao);
		glGenBuffers(1, &this->vbo);
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3), data, GL_STATIC_DRAW);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void terrain::render() {
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindVertexArray(this->vao);
	glDrawArraysInstanced(GL_PATCHES, 0, 4, MESH_SIZE * MESH_SIZE);
}

terrain::~terrain() {
	glDeleteVertexArrays(1, &this->vao);
	programglobal::oclContext->releaseCLGLTexture(this->normalMap);
	programglobal::oclContext->releaseCLGLTexture(this->heightMap);
}