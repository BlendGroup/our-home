#ifndef __CROSS_FADER__
#define __CROSS_FADER__

#include<glshaderloader.h>
#include<scenes/base.h>

class crossfader {
private:
	static glshaderprogram* programCrossFade;
	static GLuint emptyvao;
public:
	static void init();
	static void render(GLuint tex, GLfloat t);
	static void startSnapshot(GLuint& tex);
	static void endSnapshot();
	static void uninit();
};

#endif