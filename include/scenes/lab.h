#ifndef __LAB_SCENE__
#define __LAB_SCENE__

#include<scenes/base.h>
#include<scenecamera.h>

class labscene : public basescene {
public:
	void setupProgram() override;
	sceneCamera* setupCamera() override;
	void init() override;
	void render() override;
	void uninit() override;
	void keyboardfunc(int key) override;
};

#endif