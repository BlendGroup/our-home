#ifndef __GLOBAL__
#define __GLOBAL__

#include<vmath.h>
#include<clhelper.h>
#include<camera.h>
#include<shapes.h>
#include<hdr.h>

enum debugMode_t {
	NONE,
	CAMERA,
	MODEL,
	SPLINE,
	LIGHT
};

namespace programglobal {
	extern vmath::mat4 perspective;
	extern clglcontext* oclContext;
	extern camera* currentCamera;
	extern double deltaTime;
	extern shaperenderer* shapeRenderer;
	extern debugMode_t debugMode;
	extern bool isAnimating;
	extern HDR* hdr;
} // namespace programglobal

void resetFBO();

#endif