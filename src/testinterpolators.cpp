#include<iostream>
#include<memory.h>
#include<glshaderloader.h>
#include<interpolators.h>
#include<splinerenderer.h>

using namespace std;
using namespace vmath;

static glshaderprogram *testCameraProgram;

static BsplineInterpolator *bspline;
static SplineRenderer *renderer;

void setupProgramTestInterpolators(void) {
    try {
		testCameraProgram = new glshaderprogram({"src/shaders/point.vert", "src/shaders/point.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void initTestInterpolators(void) {
    vector<vec3> points;
    points.push_back(vec3(1.0f, -1.0f, 0.0f));
    points.push_back(vec3(-1.0f, 2.0f, 0.0f));
    points.push_back(vec3(1.0f, 4.0f, 0.0f));
    points.push_back(vec3(4.0f, 3.0f, 0.0f));
    points.push_back(vec3(7.0f, 5.0f, 0.0f));
    points.push_back(vec3(-9.0f, 2.0f, 0.0f));

    bspline = new BsplineInterpolator(points);
    renderer = new SplineRenderer(bspline);
}

void renderTestInterpolators(int winWidth, int winHeight) {
    mat4 projMat = perspective(45.0f, (float)winWidth / (float)winHeight, 0.01f, 100.0f);
    mat4 viewMat = translate(0.0f, 0.0f, -20.0f);
    renderer->render(viewMat, projMat, vec4(0.0f, 0.0f, 0.0f, 1.0f));
    renderer->renderPoints(viewMat, projMat);
    renderer->renderControlPoints(viewMat, projMat);
}

void uninitTestInterpolators(void) {
	if(bspline) {
		delete bspline;
		bspline = NULL;
	}
	if(testCameraProgram) {
		delete testCameraProgram;
	}
}
