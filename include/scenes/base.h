#ifndef __BASE_SCENE__
#define __BASE_SCENE__

#include<scenecamera.h>

class basescene {
public:
	virtual void setupProgram() = 0;
	virtual sceneCamera* setupCamera() = 0;
	virtual void init() = 0;
	virtual void render() = 0;
	virtual void uninit() = 0;
};

#endif