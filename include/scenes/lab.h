#ifndef __LAB_SCENE__
#define __LAB_SCENE__

#include<scenes/base.h>

class labscene : public basescene {
public:
	void setupProgram() override;
	void setupCamera() override;
	void init() override;
	void render() override;
	void uninit() override;
};

#endif