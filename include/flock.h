#ifndef __FLOCK_H__
#define __FLOCK_H__

#include<iostream>
#include<clhelper.h>
#include<vmath.h>
#include<global.h>

class Flock {
private:
    struct boid_t {
        cl_float4 position;
        cl_float4 velocity; 
    };
    size_t count;
    clglmem flockBufferCLGL;
    float cohesionRadius;
    float alignmentRadius;
    float separationRadius;
    vmath::vec3 attractorPosition;
    float maxDistanceFromAttractor;
    float maxSpeed;
    float maxForce;
public:
    Flock(size_t count);
    ~Flock();
    void update(void);
    void renderBoidsAsQuads(const vmath::mat4 &vpMatrix, float scale);
    void renderAttractorAsQuad(const vmath::mat4 &vpMatrix, float scale);
    void setCohesionRadius(float radius) {
        this->cohesionRadius = radius;
    }
    void setAttractionRadius(float radius) {
        this->alignmentRadius = radius;
    }
    void setSeparationRadius(float radius) {
        this->separationRadius = radius;
    }
    void setAttractorPosition(const vmath::vec3 &attractorPosition) {
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

using namespace std;
using namespace vmath;

Flock::Flock(size_t count) {
    this->count = count;
    this->flockBufferCLGL = programglobal::oclContext->createCLGLBuffer(count * sizeof(cl_float4), 0, CL_MEM_READ_WRITE);
    
}

#endif  // __FLOCK_H__
