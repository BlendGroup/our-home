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
	static void captureSnapshot(basescene* scenePtr, GLuint& tex);
	static void uninit();
};

#endif