#include<opensimplexnoise.h>
#include<vmath.h>
#include<clhelper.h>
#include<iostream>
#include<vector>
#include<global.h>
#include<unordered_map>
#include<errorlog.h>

using namespace vmath;
using namespace std;

GLuint opensimplexnoise::createNoiseTexture2D(vmath::ivec2 dim, vmath::ivec2 offset, float timeInterval, float amplitude, long seed) {
	cl_kernel noiseKernel = programglobal::oclContext->getKernel("noise2");
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

GLuint opensimplexnoise::createTurbulenceFBMTexture2D(vmath::ivec2 dim, vmath::ivec2 offset, float timeInterval, int octaves, float noiseoffset, long seed) {
	cl_kernel noiseKernel = programglobal::oclContext->getKernel("turbulencefbm2");
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
	programglobal::oclContext->setKernelParameters(noiseKernel, { param(0, inputGrid), param(1, outputNoiseCLGL.cl), param(2, pointCount), param(3, octaves), param(4, noiseoffset) });
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

GLuint opensimplexnoise::combineTwoNoiseTextures(GLuint inputTex1, GLuint inputTex2, ivec2 dim) {
	cl_kernel combineKernel = programglobal::oclContext->getKernel("combinetex");
	clglmem inputImage1 = programglobal::oclContext->createCLfromGLTexture(CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, inputTex1);
	clglmem inputImage2 = programglobal::oclContext->createCLfromGLTexture(CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, inputTex2);
	clglmem outputImage = programglobal::oclContext->createCLGLTexture(GL_TEXTURE_2D, GL_R32F, dim[0], dim[1], CL_MEM_WRITE_ONLY);

	programglobal::oclContext->setKernelParameters(combineKernel, {param(0, inputImage1.cl), param(1, inputImage2.cl), param(2, outputImage.cl)});
	size_t globalWorkSize[] = { (size_t)dim[0], (size_t)dim[1] };
	size_t localWorkSize[] = { 16, 16 };
	programglobal::oclContext->runCLKernel(combineKernel, 2, globalWorkSize, localWorkSize, {inputImage1, inputImage2, outputImage});

	return outputImage.gl;
}