#include<clhelper.h>
#include<fstream>
#include<vector>
#include<CL/cl_gl.h>
#include<GL/glx.h>
#include<string>
#include<errorlog.h>

using namespace std;

cl_int clhelpererr;

void handleError(cl_int errcode, string file, string line) {
	string error = "Unknown Error " + to_string(errcode);
	switch(errcode) {
	case CL_SUCCESS:return;
	case -1 : error = "CL_DEVICE_NOT_FOUND";break;
	case -2 : error = "CL_DEVICE_NOT_AVAILABLE";break;
	case -3 : error = "CL_COMPILER_NOT _AVAILABLE";break;
	case -4 : error = "CL_MEM_OBJECT _ALLOCATION_FAILURE";break;
	case -5 : error = "CL_OUT_OF_RESOURCES";break;
	case -6 : error = "CL_OUT_OF_HOST_MEMORY";break;
	case -7 : error = "CL_PROFILING_INFO_NOT _AVAILABLE";break;
	case -8 : error = "CL_MEM_COPY_OVERLAP";break;
	case -9 : error = "CL_IMAGE_FORMAT _MISMATCH";break;
	case -10: error = "CL_IMAGE_FORMAT_NOT _SUPPORTED";break;
	case -11: error = "CL_BUILD_PROGRAM _FAILURE";break;
	case -12: error = "CL_MAP_FAILURE";break;
	case -13: error = "CL_MISALIGNED_SUB _BUFFER_OFFSET";break;
	case -14: error = "CL_EXEC_STATUS_ERROR_ FOR_EVENTS_IN_WAIT_LIST";break;
	case -15: error = "CL_COMPILE_PROGRAM _FAILURE";break;
	case -16: error = "CL_LINKER_NOT_AVAILABLE";break;
	case -17: error = "CL_LINK_PROGRAM_FAILURE";break;
	case -18: error = "CL_DEVICE_PARTITION _FAILED";break;
	case -19: error = "CL_KERNEL_ARG_INFO _NOT_AVAILABLE";break;

	case -30: error = "CL_INVALID_VALUE";break;
	case -31: error = "CL_INVALID_DEVICE_TYPE";break;
	case -32: error = "CL_INVALID_PLATFORM";break;
	case -33: error = "CL_INVALID_DEVICE";break;
	case -34: error = "CL_INVALID_CONTEXT";break;
	case -35: error = "CL_INVALID_QUEUE_PROPERTIES";break;
	case -36: error = "CL_INVALID_COMMAND_QUEUE";break;
	case -37: error = "CL_INVALID_HOST_PTR";break;
	case -38: error = "CL_INVALID_MEM_OBJECT";break;
	case -39: error = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";break;
	case -40: error = "CL_INVALID_IMAGE_SIZE";break;
	case -41: error = "CL_INVALID_SAMPLER";break;
	case -42: error = "CL_INVALID_BINARY";break;
	case -43: error = "CL_INVALID_BUILD_OPTIONS";break;
	case -44: error = "CL_INVALID_PROGRAM";break;
	case -45: error = "CL_INVALID_PROGRAM_EXECUTABLE";break;
	case -46: error = "CL_INVALID_KERNEL_NAME";break;
	case -47: error = "CL_INVALID_KERNEL_DEFINITION";break;
	case -48: error = "CL_INVALID_KERNEL";break;
	case -49: error = "CL_INVALID_ARG_INDEX";break;
	case -50: error = "CL_INVALID_ARG_VALUE";break;
	case -51: error = "CL_INVALID_ARG_SIZE";break;
	case -52: error = "CL_INVALID_KERNEL_ARGS";break;
	case -53: error = "CL_INVALID_WORK_DIMENSION";break;
	case -54: error = "CL_INVALID_WORK_GROUP_SIZE";break;
	case -55: error = "CL_INVALID_WORK_ITEM_SIZE";break;
	case -56: error = "CL_INVALID_GLOBAL_OFFSET";break;
	case -57: error = "CL_INVALID_EVENT_WAIT_LIST";break;
	case -58: error = "CL_INVALID_EVENT";break;
	case -59: error = "CL_INVALID_OPERATION";break;
	case -60: error = "CL_INVALID_GL_OBJECT";break;
	case -61: error = "CL_INVALID_BUFFER_SIZE";break;
	case -62: error = "CL_INVALID_MIP_LEVEL";break;
	case -63: error = "CL_INVALID_GLOBAL_WORK_SIZE";break;
	case -64: error = "CL_INVALID_PROPERTY";break;
	case -65: error = "CL_INVALID_IMAGE_DESCRIPTOR";break;
	case -66: error = "CL_INVALID_COMPILER_OPTIONS";break;
	case -67: error = "CL_INVALID_LINKER_OPTIONS";break;
	case -68: error = "CL_INVALID_DEVICE_PARTITION_COUNT";break;
	case -69: error = "CL_INVALID_PIPE_SIZE";break;
	case -70: error = "CL_INVALID_DEVICE_QUEUE";break;
	}
	throwErr("OpenCL Error : '" + error + "' at line '" + line + "' in file '" + file + "'");
}

clglcontext::clglcontext(int platformNo, int deviceNo) {
	try {
		cl_platform_id *platforms;
		cl_device_id *devices;
		cl_uint num;
		
		CLErr(clhelpererr = clGetPlatformIDs(0, NULL, &num));
		if(num >= platformNo) {
			platformNo = 0;
		}
		platforms = (cl_platform_id*)alloca(sizeof(cl_platform_id) * num);
		CLErr(clhelpererr = clGetPlatformIDs(num, platforms, NULL));
		

		// char buffer[1024];
		// clGetPlatformInfo(platforms[platformNo], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL);

		CLErr(clhelpererr = clGetDeviceIDs(platforms[platformNo], CL_DEVICE_TYPE_ALL, 0, NULL, &num));
		if(num >= deviceNo) {
			deviceNo = 0;
		}
		devices = (cl_device_id*)alloca(sizeof(cl_device_id) * num);
		CLErr(clhelpererr = clGetDeviceIDs(platforms[platformNo], CL_DEVICE_TYPE_ALL, num, devices, NULL));

		this->device = devices[deviceNo];
		
		cl_context_properties contextProp[] = {
			CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
			CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
			CL_CONTEXT_PLATFORM, (cl_context_properties)platforms[platformNo],
			0
		};
		CLErr(this->context = clCreateContext(contextProp, 1, &devices[deviceNo], NULL, NULL, &clhelpererr));

		CLErr(this->cmdQueue = clCreateCommandQueue(this->context, devices[deviceNo], 0, &clhelpererr));
	} catch(string errString) {
		throwErr(errString);
	}
}

void clglcontext::compilePrograms(vector<string> programNames) {
	try {
		cl_uint num;
		char **src = (char**)alloca(sizeof(char*) * programNames.size());
		size_t *len = (size_t*)alloca(sizeof(size_t) * programNames.size());
		string *strs = new string[programNames.size()];

		for(int i = 0; i < programNames.size(); i++) {
			ifstream prog(programNames[i]);
			if(prog.fail()) {
				// errorString<<"Error: file '"<<programNames[i]<<"' couldn't be opened"<<endl;
			}
			string programSrc((istreambuf_iterator<char>(prog)), istreambuf_iterator<char>());
			strs[i] = programSrc;
			src[i] = (char*)strs[i].c_str();
			len[i] = strs[i].length();
		}
		CLErr(cl_program program = clCreateProgramWithSource(this->context, programNames.size(), (const char**)src, len, &clhelpererr));
		CLErr(clhelpererr = clBuildProgram(program, 1, &this->device, NULL, NULL, NULL));
		if(clhelpererr != CL_SUCCESS) {
			char *buffer;
			size_t s;
			clhelpererr = clGetProgramBuildInfo(program, this->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &s);
			buffer = (char*)alloca(s);
			clhelpererr = clGetProgramBuildInfo(program, this->device, CL_PROGRAM_BUILD_LOG, s, buffer, NULL);
			// errorString<<"Error:"<<buffer<<endl;
		}
		CLErr(clhelpererr = clCreateKernelsInProgram(program, 0, NULL, &num));
		cl_kernel *kernels = (cl_kernel*)alloca(sizeof(cl_kernel) * num);
		clhelpererr = clCreateKernelsInProgram(program, num, kernels, NULL);
		for(int i = 0; i < num; i++) {
			size_t size;
			char *buffer;
			CLErr(clhelpererr = clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, 0, NULL, &size));
			buffer = (char*)malloc(size);
			clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, size, buffer, NULL);
			this->kernels[string(buffer)] = kernels[i];
		}
		delete[] strs;
		clReleaseProgram(program);
	} catch(string errString) {
		throwErr(errString);
	}
}

cl_kernel clglcontext::getKernel(std::string name) {
	if(this->kernels.count(name) == 0) {
		throwErr("kernel named : '" + name + "'" + " not found.");
		return NULL;
	}
	return this->kernels[name];
}

void clglcontext::setKernelParameters(cl_kernel kernel, vector<clkernelparamater> kernelList) {
	try {
		cl_int err;
		for(int i = 0; i < kernelList.size(); i++) {
			CLErr(clhelpererr = clSetKernelArg(kernel, kernelList[i].position, kernelList[i].size, kernelList[i].param));
		}
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void clglcontext::setKernelParameters(string kernelName, vector<clkernelparamater> kernelList) {
	cl_int err;
	if(this->kernels.count(kernelName) == 0) {
		throwErr("kernel named : '" + kernelName + "'" + " not found.");
		return;
	}
	try {
		cl_kernel kernel = this->kernels[kernelName];
		for(int i = 0; i < kernelList.size(); i++) {
			CLErr(clhelpererr = clSetKernelArg(kernel, kernelList[i].position, kernelList[i].size, kernelList[i].param));
		}
	} catch(string errorString) {
		throwErr(errorString);
	}
}

cl_mem clglcontext::createGLCLBuffer(cl_mem_flags memFlags, GLuint buffer) {
	try {
		cl_mem mem;
		CLErr(mem = clCreateFromGLBuffer(this->context, memFlags, buffer, &clhelpererr));
		return mem;
	} catch(string errorString) {
		throwErr(errorString);
		return NULL;
	}
}

cl_mem clglcontext::createGLCLTexture(cl_mem_flags memFlags, GLenum texTarget, GLuint mipMapLevel, GLuint texture) {
	try {
		cl_mem mem;
		CLErr(mem = clCreateFromGLTexture(this->context, memFlags, texTarget, mipMapLevel, texture, &clhelpererr));
		return mem;
	} catch(string errorString) {
		throwErr(errorString);
		return NULL;
	}
}

void clglcontext::runCLKernel(cl_kernel kernel, cl_uint workDim, size_t *globalSize, size_t *localSize, vector<cl_mem> globjects) {
	try {
		CLErr(clhelpererr = clEnqueueNDRangeKernel(this->cmdQueue, kernel, workDim, NULL, globalSize, localSize, 0, NULL, NULL));
	} catch(string errorString) {
		throwErr(errorString);
	}
}

clglcontext::~clglcontext() {
	for(std::pair<std::string, cl_kernel> kernelPair : this->kernels) {
		clReleaseKernel(kernelPair.second);
	}
	clReleaseCommandQueue(this->cmdQueue);
	clReleaseContext(this->context);
	clReleaseDevice(this->device);
}