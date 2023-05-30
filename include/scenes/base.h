#ifndef __BASE_SCENE__
#define __BASE_SCENE__

#include<camera.h>

#define tex_1k 1920, 1080

class basescene {
protected:
public:
	virtual void setupProgram() = 0;
	virtual void setupCamera() = 0;
	virtual void init() = 0;
	virtual void render() = 0;
	virtual void update() = 0;
	virtual void reset() = 0;
	virtual void uninit() = 0;
	virtual void keyboardfunc(int key) = 0;
	virtual camera* getCamera() = 0;
	virtual void crossfade() = 0;
};

void playNextScene();

#endif