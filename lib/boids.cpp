#include <boids.h>

using namespace std;
using namespace vmath;

Boid::Boid(vec3 position, vec3 velocity) {
    m_position = position;
    m_velocity = velocity;
    m_acceleration = vec3(0.0f, 0.0f, 0.0f);
}

void Boid::update(const vector<Boid> *flock, const vec3 &attractor_pos) {
    vec3 force = cohesion(flock, PERCEPTION_RADIUS_COHESION) +
        alignment(flock, PERCEPTION_RADIUS_ALIGNMENT) +
        separation(flock, PERCEPTION_RADIUS_SEPARATION) +
        attraction(attractor_pos);
    m_acceleration = force;
    m_position += m_velocity;
    m_velocity += m_acceleration;

    /* set bounds on particle positions */
    if(length(m_position - attractor_pos) > 50.0f) {
        m_velocity = -m_velocity;
    }

    /* set bounds on particle speeds */
    if(length(m_velocity) > MAX_SPEED) {
        m_velocity = normalize(m_velocity) * MAX_SPEED;
    }

    /* reset force on particle */
    m_acceleration = vec3(0.0f, 0.0f, 0.0f);
}

vec3 Boid::alignment(const vector<Boid> *flock, float perceptionRadius) {
    vec3 force(0.0f, 0.0f, 0.0f);
    const float epsilon = 10e-6;
    int count = 0;
    for(Boid other : *flock) {
        float alignment = distance(other.m_velocity, m_velocity);
        if((alignment > epsilon) && (alignment <= perceptionRadius)) {
            force += other.m_velocity;
            count++;
        }
    }
    if(count > 0) {
        force = force / (float)count;
    }
    if(length(force) > MAX_FORCE) {
        force = normalize(force) * MAX_FORCE;
    }
    return force;
}

vec3 Boid::cohesion(const vector<Boid> *flock, float perceptionRadius) {
    vec3 force(0.0f, 0.0f, 0.0f);
    const float epsilon = 10e-6;
    int count = 0;
    for(Boid other : *flock) {
        float dist = distance(other.m_position, m_position);
        if((dist > epsilon) && (dist <= perceptionRadius)) {
            force += other.m_position;
            count++;
        }
    }
    if(count > 0) {
        force = force / (float)count;
    }
    if(length(force) > MAX_FORCE) {
        force = normalize(force) * MAX_FORCE;
    }
    return force;
}

vec3 Boid::separation(const vector<Boid> *flock, float perceptionRadius) {
    vec3 force(0.0f, 0.0f, 0.0f);
    const float epsilon = 10e-6;
    int count = 0;
    for(Boid other : *flock) {
        float dist = distance(other.m_position, m_position);
        if((dist > epsilon) && (dist <= perceptionRadius)) {
            force += normalize(m_position - other.m_position) / dist;
            count++;
        }
    }
    if(count > 0) {
        force = force / (float)count;
    }
    if(length(force) > 0.0f) {
        force = normalize(force) * MAX_SPEED;
        force -= m_velocity;
        if(length(force) > MAX_FORCE) {
            force = normalize(force) * MAX_FORCE;
        }
    }
    return force;
}

vec3 Boid::attraction(const vmath::vec3 &attractor_pos) {
    vec3 force(0.0f, 0.0f, 0.0f);
    force = (attractor_pos - m_position) / distance(attractor_pos, m_position);
    if(length(force) > MAX_FORCE) {
        force = normalize(force) * MAX_FORCE;
    }
    return force;
}

vec3 Boid::position(void) {
    return m_position;
}
