#ifndef __OPENCLDL__
#define __OPENCLDL__

#define CL_TARGET_OPENCL_VERSION 120
#include<GL/glew.h>
#include<GL/gl.h>
#include<CL/cl.h>
#include<CL/cl_gl.h>
#include<vector>
#include<iostream>
#include<unordered_map>

extern cl_int clhelpererr;

#define clkernelparam(pos, mem) {pos, &mem, sizeof(mem)}

struct clglmem {
	GLuint gl;
	cl_mem cl;
};

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
	clglmem createCLfromGLBuffer(cl_mem_flags memFlags, GLuint buffer);
	clglmem createCLfromGLTexture(cl_mem_flags memFlags, GLenum texTarget, GLuint mipMapLevel, GLuint texture);
	clglmem createCLGLBuffer(GLsizei size, GLbitfield glflags, cl_mem_flags clflags);
	clglmem createCLGLTexture(GLenum texTarget, GLenum format, GLsizei width, GLsizei height, cl_mem_flags clflags);
	void runCLKernel(cl_kernel kernel, cl_uint workDims, size_t *globalSize, size_t *localSize, std::vector<clglmem> globjects);
	void printKernelList(std::ostream& out);
	cl_command_queue getCommandQueue(void);
	cl_context getContext(void);
	void releaseCLGLBuffer(clglmem& clglmem);
	void releaseCLGLTexture(clglmem& clglmem);
	~clglcontext();
};

void handleError(cl_int errcode, std::string line, std::string filename);

#define CLErr(function) function;handleError(clhelpererr, #function, __FILE__)

#endif