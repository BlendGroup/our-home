#ifndef __GLOBAL__
#define __GLOBAL__

#include<vmath.h>
#include<clhelper.h>
#include<opensimplexnoise.h>
#include<camera.h>
#include<shapes.h>

namespace programglobal {
	extern vmath::mat4 perspective;
	extern clglcontext* oclContext;
	extern camera* currentCamera;
	extern opensimplexnoise* noiseGenerator;
	extern double deltaTime;
	extern shaperenderer* shapeRenderer;
} // namespace programglobal

#endif