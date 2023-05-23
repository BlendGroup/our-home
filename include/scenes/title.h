#ifndef __TITLE_SCENE__
#define __TITLE_SCENE__

#include<scenes/base.h>
#include<scenecamera.h>

class titlescene : public basescene {
public:
	void setupProgram() override;
	sceneCamera* setupCamera() override;
	void init() override;
	void render() override;
	void update(sceneCamera* cam) override;
	void uninit() override;
	void keyboardfunc(int key) override;
};

#endif