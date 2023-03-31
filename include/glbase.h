#ifndef __GLBASE__
#define __GLBASE__

class glbase {
public:
	virtual void setupProgram(void) = 0;
	virtual void init(void) = 0;
	virtual void render(void) = 0;
	virtual void uninit(void) = 0;
	virtual void keyboardfunc(int key) = 0;
};

#endif