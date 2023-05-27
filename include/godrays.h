#ifndef __GODRAYS__
#define __GODRAYS__

#include<string>
#include<vmath.h>
#include<glshaderloader.h>
#include<errorlog.h>

class godrays {
private:
    glshaderprogram *godraysProgram;
	vmath::mat4 mvpMatrix;
	vmath::vec4 pos;
	GLfloat density;
	GLfloat weight;
	GLfloat decay;
	GLfloat exposure;
	int samples;
	GLuint occlusionTex;
	vec2 sscoord;
public:
	godrays();
	~godrays();
	void setDensity(GLfloat density);
	void setWeight(GLfloat weight);
	void setDecay(GLfloat decay);
	void setExposure(GLfloat exposure);
	void setSamples(int samples);
	void setOcclusionTex(GLuint occlusionTex);
	void setScreenSpaceCoords(const vmath::mat4& mvp, const vmath::vec4& pos);
	void renderRays();
};

#endif // __GODRAYS__
