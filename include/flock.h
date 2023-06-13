#ifndef __FLOCK_H__
#define __FLOCK_H__

#include<clhelper.h>
#include<glshaderloader.h>
#include<vmath.h>
#include<global.h>
#include<sphere.h>

class Flock {
private:
    struct boid_t {
        vmath::vec4 position;
        vmath::vec4 velocity; 
    };
    cl_uint count;
    clglmem flockBufferCLGL;
    vmath::vec3 attractorPosition;
    glshaderprogram *flockProgram;
    glshaderprogram *colorProgram;
    sphere *boidSphere;
    float cohesionRadius;
    float alignmentRadius;
    float separationRadius;
    float attractionWeight;
    float maxSpeed;
    float maxForce;
public:
    Flock(size_t count, const vmath::vec3 &initAttractorPosition);
    ~Flock();
    void update(void);
    void renderAsSpheres(const vmath::vec4 &color, const vmath::vec4 &emissive, float scale);
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
    void setAttractorPosition(const vmath::vec3 &attractorPosition) {
        this->attractorPosition = attractorPosition;
    }
    void setAttractionWeight(float weight) {
        this->attractionWeight = weight;
    }
    void setMaxSpeed(float speed) {
        this->maxSpeed = speed;
    }
    void setMaxForce(float force) {
        this->maxForce = force;
    }
};

#endif  // __FLOCK_H__
