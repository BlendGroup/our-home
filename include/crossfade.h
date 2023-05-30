#ifndef __CROSS_FADER__
#define __CROSS_FADER__

#include<glshaderloader.h>

class crossfader {
private:
	static glshaderprogram* programCrossFade;
	static GLuint emptyvao;
public:
	static void init();
	static void render(GLuint tex, GLfloat t);
	static void uninit();
};

#endif