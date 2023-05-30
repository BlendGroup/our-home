#ifndef __CAMERA__
#define __CAMERA__

#include<vmath.h>

class camera {
public:
	virtual vmath::mat4 matrix(void) const = 0;
	virtual vmath::mat4 matrixYFlippedOnPlane(float planeY) const = 0;
	virtual vmath::vec3 position(void) const = 0;
};

#endif