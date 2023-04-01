#ifndef __TEST_MODEL__
#define __TEST_MODEL__

#include "vmath.h"
#include <string>

void setupProgramTestModel();
void initTestModel(std::string path,bool gStatic);
void renderTestModel(vmath::mat4 perspective);
void uninitTestModel();
void keyboardfuncModel(int key);
#endif