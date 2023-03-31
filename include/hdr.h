#ifndef __HDR__
#define __HDR__

#include"glbase.h"
#include<GL/glew.h>

class HDR : public glbase {
private:
    GLuint FBO;
    GLuint RBO;
    GLuint Tex;
    GLfloat exposure;
    GLfloat fade;
	GLsizei size;
public:
	HDR(GLfloat exposure, GLfloat fade, GLsizei size);
	void setupProgram(void) override;
	void init(void) override;
	void render(void) override;
	void uninit(void) override;
	void keyboardfunc(int key) override;
	GLuint getFBO(void);
	GLsizei getSize(void);
	GLfloat getExposure(void);
	void updateExposure(GLfloat delta);
};

#endif