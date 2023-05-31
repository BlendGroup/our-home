#ifndef __TEST_AS__
#define __TEST_AS__

#include<camera.h>

void setupProgramAS();
void initAS();
void renderAS(camera* cam,vmath::vec3 camPos);
void uninitAS();
#endif