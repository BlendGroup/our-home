#include<testLake.h>
#include<lake.h>
#include<vmath.h>

using namespace vmath;

lake* basicLake;

void setupProgramTestLake() {
	basicLake->setupProgram();
}

void initTestLake() {
	basicLake = new lake(mat4::identity());

	basicLake->init();
}

void renderTestLake() {
	basicLake->render();
}

void uninitTestLake() {
	basicLake->uninit();
	delete basicLake;
}