#include<terrain.h>
#include<errorlog.h>
#include<iostream>
#include<global.h>
#include<vmath.h>

using namespace std;
using namespace vmath;

#define TEXTURE_SIZE 512
#define MESH_SIZE 256

terrain::terrain(GLuint heightMap, bool calcNormal, GLfloat minTess, GLfloat maxTess) {
	try {
		this->maxTess = maxTess;
		this->minTess = minTess;
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

		struct vertex_t {
			vec3 position;
			vec2 texcoord;
		} *data = new vertex_t[MESH_SIZE * MESH_SIZE * 4];
		int k = 0;

		float mesh_start = (float)(MESH_SIZE - 1) / 2.0f;
		for(int i = 0; i < MESH_SIZE; i++) {
			for(int j = 0; j < MESH_SIZE; j++) {
				vec3 meshCenter = vec3(-mesh_start + ((float)i / (float)(MESH_SIZE - 1)) * (MESH_SIZE - 1), 0.0f, -mesh_start + ((float)j / (float)(MESH_SIZE-1)) * (MESH_SIZE - 1));
				vec2 meshTexCenter = vec2((float(i) / (float)(MESH_SIZE - 1)), (float(j) / (float)(MESH_SIZE - 1)));
				data[k].position = meshCenter + vec3(0.5f, 0.0f, 0.5f);
				data[k].texcoord = vec2(data[k].position[0] / MESH_SIZE + 0.5f, data[k].position[2] / MESH_SIZE + 0.5f);
				k++;
				data[k].position = meshCenter + vec3(-0.5f, 0.0f, 0.5f);
				data[k].texcoord = vec2(data[k].position[0] / MESH_SIZE + 0.5f, data[k].position[2] / MESH_SIZE + 0.5f);
				k++;
				data[k].position = meshCenter + vec3(0.5f, 0.0f, -0.5f);
				data[k].texcoord = vec2(data[k].position[0] / MESH_SIZE + 0.5f, data[k].position[2] / MESH_SIZE + 0.5f);
				k++;
				data[k].position = meshCenter + vec3(-0.5f, 0.0f, -0.5f);
				data[k].texcoord = vec2(data[k].position[0] / MESH_SIZE + 0.5f, data[k].position[2] / MESH_SIZE + 0.5f);
				k++;
			}
		}

		glCreateVertexArrays(1, &this->vao);
		glBindVertexArray(this->vao);
		glGenBuffers(1, &this->vbo);
		glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * MESH_SIZE * MESH_SIZE * 4, data, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, position));
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, texcoord));
		glEnableVertexAttribArray(1);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void terrain::render() {
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	glBindVertexArray(this->vao);
	glDrawArrays(GL_PATCHES, 0, 4 *MESH_SIZE * MESH_SIZE);
}

terrain::~terrain() {
	glDeleteVertexArrays(1, &this->vao);
	programglobal::oclContext->releaseCLGLTexture(this->normalMap);
	programglobal::oclContext->releaseCLGLTexture(this->heightMap);
}