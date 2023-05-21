#ifndef __TEST_GODRAYS__
#define __TEST_GODRAYS__

#include<iostream>
#include<global.h>
#include<glshaderloader.h>
#include<glLight.h>
#include<vmath.h>
#include<errorlog.h>

void setupProgramTestGodrays(int winWidth, int winHeight);
void initTestGodrays(void);
void renderTestGodrays(camera *cam);
void uninitTestGodrays(void);

#endif // __TEST_GODRAYS__
