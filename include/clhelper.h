#ifndef __OPENCLDL__
#define __OPENCLDL__

#define CL_TARGET_OPENCL_VERSION 120
#include<CL/cl.h>
#include<vector>
#include<iostream>
#include<GL/gl.h>
#include<unordered_map>

extern cl_int clhelpererr;

#define param(p, x) {p, &x, sizeof(x)}

struct clkernelparamater {
	int position;
	void *param;
	size_t size;
};

class clglcontext {
private:
	cl_context context;
	cl_device_id device;
	cl_command_queue cmdQueue;
	std::unordered_map<std::string, cl_kernel> kernels;
public:
	clglcontext(int platformNo = 0, int deviceNo = 0);
	void compilePrograms(std::vector<std::string> programNames);
	cl_kernel getKernel(std::string name);
	void setKernelParameters(cl_kernel kernel, std::vector<clkernelparamater> kernelList);
	void setKernelParameters(std::string kernelName, std::vector<clkernelparamater> kernelList);
	cl_mem createGLCLBuffer(cl_mem_flags memFlags, GLuint buffer);
	cl_mem createGLCLTexture(cl_mem_flags memFlags, GLenum texTarget, GLuint mipMapLevel, GLuint texture);
	void runCLKernel(cl_kernel kernel, cl_uint workDims, size_t *globalSize, size_t *localSize, std::vector<cl_mem> globjects);
	~clglcontext();
};

void handleError(cl_int errcode, std::string line, std::string filename);

#define CLErr(function) function;handleError(clhelpererr, #function, __FILE__)

#endif