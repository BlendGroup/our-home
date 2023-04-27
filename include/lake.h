#ifndef __LAKE__
#define __LAKE__

#include<glbase.h>

class lake : public glbase {
private:
	GLuint vao;
public:
	lake();
	void setupProgram(void) override;
	void init(void) override;
	void render(void) override;
	void uninit(void) override;
	~lake();
};

#endif