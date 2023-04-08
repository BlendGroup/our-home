#ifndef __DEBUG_CAMERA__
#define __DEBUG_CAMERA__

#include"camera.h"
#include"vmath.h"

class debugCamera : public camera {
private:
	vmath::vec3 position;
	vmath::vec3 front;
	vmath::vec3 up;
	float pitch;
	float yaw;
public:
	debugCamera(vmath::vec3 position, float yaw, float pitch);
	vmath::mat4 matrix(void) override;
	void keyboardFunc(unsigned int key);
	void mouseFunc(int action, int x, int y);
};

#endif