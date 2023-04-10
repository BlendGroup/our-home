#ifndef __CAMERA__
#define __CAMERA__

#include<vmath.h>

class camera {
public:
	virtual vmath::mat4 matrix(void) const = 0;
};

#endif