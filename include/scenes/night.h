#ifndef __DAY_SCENE__
#define __DAY_SCENE__

#include<scenes/base.h>

class nightscene : public basescene {
public:
	void setupProgram() override;
	void setupCamera() override;
	void init() override;
	void render() override;
	void update() override;
	void reset() override;
	void uninit() override;
	void keyboardfunc(int key) override;
	camera* getCamera() override;
	void crossfade() override;
};

#endif