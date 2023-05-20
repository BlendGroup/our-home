#include<opensimplexnoise.h>
#include<vmath.h>
#include<clhelper.h>
#include<iostream>
#include<vector>
#include<global.h>
#include<unordered_map>
#include<errorlog.h>

#define PSIZE 2048
#define NUM_LATTICE 8
#define NUM_POINTS_PER_2D_LATICE 4
#define NUM_POINTS_PER_3D_LATICE 14

using namespace vmath;
using namespace std;

#define PROGRAM_NAME "opensimplexnoise.cl"
#define ERROR_INVALID_NOISE "Invalid Noise Type"

opensimplexnoise::opensimplexnoise() {
	try {
		string programName;
		programglobal::oclContext->compilePrograms({"shaders/opensimplexnoise.cl"});
	} catch(string errString) {
		throwErr(errString);
	}
}

void create2DNoiseTexture(cl_kernel kernel, cl_mem inputGrid, clglmem outputNoise, ivec2 dim, float amplitude, long seed) {
	try {
	} catch(string errorString) {
		throwErr(errorString);
	}
}

GLuint opensimplexnoise::createNoiseTexture2D(vmath::ivec2 dim, vmath::ivec2 offset, float timeInterval, float amplitude, long seed) {
	cl_kernel noiseKernel = programglobal::oclContext->getKernel("noise2");
	// programglobal::oclContext->printKernelList(cout);
	cl_mem inputGrid;
	const float freq = 1.0f / timeInterval; 
	size_t num_points = dim[0] * dim[1];
	vec2 *grid = (vec2*)malloc(num_points * sizeof(vec2));
	int i = 0;
	for (int y = 0; y < dim[1]; y++){
		for (int x = 0; x < dim[0]; x++) {
			grid[i++] = vec2((x + offset[0]) * freq, (y + offset[1]) * freq);
		}
	}
	CLErr(inputGrid = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(vec2) * num_points, grid, &clhelpererr));
	GLuint outputNoise;
	cl_uint pointCount = dim[0] * dim[1];
	size_t globalWorkSize = pointCount;
	clglmem outputNoiseCLGL;
	outputNoiseCLGL = programglobal::oclContext->createCLGLBuffer(sizeof(float) * dim[0] * dim[1], GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, CL_MEM_READ_WRITE);
	programglobal::oclContext->setKernelParameters(noiseKernel, { param(0, inputGrid), param(1, outputNoiseCLGL.cl), param(2, pointCount), param(3, amplitude) });
	programglobal::oclContext->runCLKernel(noiseKernel, 1, &globalWorkSize, NULL, { outputNoiseCLGL });
	glGenTextures(1, &outputNoise);
	glBindTexture(GL_TEXTURE_2D, outputNoise);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, outputNoiseCLGL.gl);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dim[0], dim[1], 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	clReleaseMemObject(inputGrid);
	return outputNoise;
}

GLuint opensimplexnoise::createFBMTexture2D(vmath::ivec2 dim, vmath::ivec2 offset, float timeInterval, int octaves, long seed) {
	cl_kernel noiseKernel = programglobal::oclContext->getKernel("fbm2");
	// programglobal::oclContext->printKernelList(cout);
	cl_mem inputGrid;
	const float freq = 1.0f / timeInterval; 
	size_t num_points = dim[0] * dim[1];
	vec2 *grid = (vec2*)malloc(num_points * sizeof(vec2));
	int i = 0;
	for (int y = 0; y < dim[1]; y++){
		for (int x = 0; x < dim[0]; x++) {
			grid[i++] = vec2((x + offset[0]) * freq, (y + offset[1]) * freq);
		}
	}
	CLErr(inputGrid = clCreateBuffer(programglobal::oclContext->getContext(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(vec2) * num_points, grid, &clhelpererr));
	GLuint outputNoise;
	cl_uint pointCount = dim[0] * dim[1];
	size_t globalWorkSize = pointCount;
	clglmem outputNoiseCLGL;
	outputNoiseCLGL = programglobal::oclContext->createCLGLBuffer(sizeof(float) * dim[0] * dim[1], GL_MAP_WRITE_BIT | GL_MAP_READ_BIT, CL_MEM_READ_WRITE);
	programglobal::oclContext->setKernelParameters(noiseKernel, { param(0, inputGrid), param(1, outputNoiseCLGL.cl), param(2, pointCount), param(3, octaves) });
	programglobal::oclContext->runCLKernel(noiseKernel, 1, &globalWorkSize, NULL, { outputNoiseCLGL });
	glGenTextures(1, &outputNoise);
	glBindTexture(GL_TEXTURE_2D, outputNoise);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, outputNoiseCLGL.gl);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, dim[0], dim[1], 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	clReleaseMemObject(inputGrid);
	return outputNoise;
}
