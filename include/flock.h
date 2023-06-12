#ifndef __FLOCK_H__
#define __FLOCK_H__

#include<clhelper.h>
#include<glshaderloader.h>
#include<vmath.h>
#include<global.h>

class Flock {
private:
    struct boid_t {
        vmath::vec4 position;
        vmath::vec4 velocity; 
    };
    cl_uint count;
    clglmem flockBufferCLGL;
    vmath::vec4 attractorPosition;
    GLuint emptyVao;
    glshaderprogram *flockProgram;
    glshaderprogram *colorProgram;
    float cohesionRadius;
    float alignmentRadius;
    float separationRadius;
    float maxDistanceFromAttractor;
    float maxSpeed;
    float maxForce;
public:
    Flock(size_t count, const vmath::vec4 &initAttractorPosition);
    ~Flock();
    void update(void);
    void renderAsQuads(const vmath::vec4 &color, const vmath::vec4 &emissive, float scale);
    void renderAttractorAsQuad(const vmath::vec4 &color, const vmath::vec4 &emissive, float scale);
    void setCohesionRadius(float radius) {
        this->cohesionRadius = radius;
    }
    void setAttractionRadius(float radius) {
        this->alignmentRadius = radius;
    }
    void setSeparationRadius(float radius) {
        this->separationRadius = radius;
    }
    void setAttractorPosition(const vmath::vec4 &attractorPosition) {
        this->attractorPosition = attractorPosition;
    }
    void setMaxDistanceFromAttractor(float distance) {
        this->maxDistanceFromAttractor = distance;
    }
    void setMaxSpeed(float speed) {
        this->maxSpeed = speed;
    }
    void setMaxForce(float force) {
        this->maxForce = force;
    }
};

#endif  // __FLOCK_H__
