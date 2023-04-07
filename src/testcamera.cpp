#include<iostream>
#include<memory.h>
#include"../include/glshaderloader.h"
#include"../include/interpolators.h"
#include"../include/splinerenderer.h"

using namespace std;
using namespace vmath;

static glshaderprogram *testCameraProgram;
static struct test_camera_vaos {
	GLuint plane;
	GLuint cube;
} vaos;
static struct test_camera_vbos {
	GLuint plane;
	GLuint cube;
} vbos;

static BsplineInterpolator *bspline;
static SplineRenderer *renderer;

void setupProgramTestCamera(void) {
    try {
		testCameraProgram = new glshaderprogram({"src/shaders/point.vert", "src/shaders/point.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void initTestCamera(void) {
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

void renderTestCamera(int winWidth, int winHeight) {
    mat4 projMat = perspective(45.0f, (float)winWidth / (float)winHeight, 0.01f, 100.0f);
    mat4 viewMat = translate(0.0f, 0.0f, -20.0f);
    renderer->render(viewMat, projMat, vec4(0.0f, 0.0f, 0.0f, 1.0f));
    renderer->renderPoints(viewMat, projMat);
    renderer->renderControlPoints(viewMat, projMat);
}

void uninitTestCamera(void) {
	if(vbos.cube) {
		glDeleteBuffers(1, &vbos.cube);
		vbos.cube = 0;
	}
	if(vaos.cube) {
		glDeleteVertexArrays(1, &vaos.cube);
		vaos.cube = 0;
	}
	if(vbos.plane) {
		glDeleteBuffers(1, &vbos.plane);
		vbos.plane = 0;
	}
	if(vaos.plane) {
		glDeleteVertexArrays(1, &vaos.plane);
		vaos.plane = 0;
	}
	if(bspline) {
		delete bspline;
		bspline = NULL;
	}
	if(testCameraProgram) {
		delete testCameraProgram;
	}
}
