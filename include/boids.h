#ifndef __BOIDS_H__
#define __BOIDS_H__

#include <iostream>
#include <vector>
#include<clhelper.h>
#include "vmath.h"

#define MAX_FORCE 0.8f
#define MAX_SPEED 0.4f
#define PERCEPTION_RADIUS_COHESION 1.5f
#define PERCEPTION_RADIUS_ALIGNMENT 5.0f
#define PERCEPTION_RADIUS_SEPARATION 2.0f

class Boid {
private:
    vmath::vec3 m_position;
    vmath::vec3 m_velocity;
    vmath::vec3 m_acceleration;

public:
    Boid(vmath::vec3 position, vmath::vec3 velocity);
    void update(const std::vector<Boid> *flock, const vmath::vec3 &attractor_pos);
    vmath::vec3 alignment(const std::vector<Boid> *flock, float perceptionRadius);
    vmath::vec3 cohesion(const std::vector<Boid> *flock, float perceptionRadius);
    vmath::vec3 separation(const std::vector<Boid> *flock, float perceptionRadius);
    vmath::vec3 attraction(const vmath::vec3 &attractor_pos);
    vmath::vec3 position(void);
};

#endif  // __BOIDS_H__
