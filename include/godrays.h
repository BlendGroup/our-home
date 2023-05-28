#ifndef __GODRAYS__
#define __GODRAYS__

#include<string>
#include<vmath.h>
#include<glshaderloader.h>
#include<errorlog.h>
#include<hdr.h>

class godrays {
private:
    glshaderprogram *godraysProgram;
	GLfloat density;
	GLfloat weight;
	GLfloat decay;
	GLfloat exposure;
	int samples;
	vmath::vec2 sscoord;
	GLuint emptyvao;
public:
	godrays();
	~godrays();
	void setDensity(GLfloat density);
	void setWeight(GLfloat weight);
	void setDecay(GLfloat decay);
	void setExposure(GLfloat exposure);
	void setSamples(int samples);
	void setScreenSpaceCoords(const vmath::mat4& mvp, const vmath::vec4& pos);
	void renderRays(HDR* hdr);
};

#endif // __GODRAYS__
