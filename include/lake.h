#ifndef __LAKE__
#define __LAKE__

#include<glbase.h>
#include<vmath.h>
#include<glshaderloader.h>

class lake : public glbase {
private:
	glshaderprogram* renderLake;
	vmath::mat4 modelMatrix;
	GLuint vao;
public:
	lake(vmath::mat4);
	void setupProgram(void) override;
	void init(void) override;
	void render(void) override;
	void uninit(void) override;
	~lake();
};

#endif