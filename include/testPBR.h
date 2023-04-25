#ifndef __TEST_PBR__
#define __TEST_PBR__

#include<camera.h>

void setupProgramTestPbr();
void initTestPbr();
void renderTestPbr(camera* cam,vmath::vec3 camPos);
void uninitTestPbr();

#endif