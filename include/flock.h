#ifndef __FLOCK_H__
#define __FLOCK_H__

#include<glshaderloader.h>
#include<vmath.h>
#include<global.h>
#include<sphere.h>
#include<GL/gl.h>

class Flock {
private:
    struct flock_member {
		vmath::vec3 position;
		unsigned int : 32;
		vmath::vec3 velocity;
		unsigned int : 32;
	};
	unsigned count;
    vmath::vec3 attractorPosition;
    glshaderprogram *flockUpdateProgram;
	glshaderprogram *flockProgram;
    glshaderprogram *colorProgram;
    sphere *boidSphere;
	GLuint flockBuffer[2];
	int frameIndex;
public:
    Flock(size_t count, const vmath::vec3 &initAttractorPosition);
    ~Flock();
    void update(void);
    void renderAsSpheres(const vmath::mat4& mMat, const vmath::vec4 &color, const vmath::vec4 &emissive, float scale);
    void renderAttractorAsQuad(const vmath::mat4& mMat, const vmath::vec4 &color, const vmath::vec4 &emissive, float scale);
    void setAttractorPosition(const vmath::vec3 &attractorPosition) {
        this->attractorPosition = attractorPosition;
    }
};

#endif  // __FLOCK_H__
