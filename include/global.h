#ifndef __GLOBAL__
#define __GLOBAL__

#include<vmath.h>
#include<clhelper.h>
#include<opensimplexnoise.h>
#include<camera.h>

namespace programglobal {
	extern vmath::mat4 perspective;
	extern clglcontext* oclContext;
	extern camera* currentCamera;
	extern opensimplexnoise* noiseGenerator;
} // namespace programglobal

#endif