#ifndef __CAMERA__
#define __CAMERA__

#include"../include/vmath.h"

class camera {
public:
	virtual vmath::mat4 matrix(void) = 0;
};

#endif