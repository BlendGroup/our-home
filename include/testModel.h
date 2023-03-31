#ifndef __TEST_MODEL__
#define __TEST_MODEL__

#include "vmath.h"
#include <string>

void setupProgramTestModel();
void initTestModel(std::string path);
void renderTestModel(vmath::mat4 perspective);
void uninitTestModel();
#endif