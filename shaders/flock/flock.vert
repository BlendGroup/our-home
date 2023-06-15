#version 460 core

#define MAX_BOIDS_PER_FLOCK 1024

layout(location = 0) in vec4 vPos;

struct boid_t {
    vec3 position;
    vec3 velocity;
};
layout(std140) uniform FlockBlock {
    boid_t flock_buffer[MAX_BOIDS_PER_FLOCK];
};

uniform mat4 mMat;
uniform mat4 vMat;
uniform mat4 pMat;
uniform float scale;

void main(void) {
    mat4 lmMat = mat4(
        vec4(scale, 0.0, 0.0, 0.0),
        vec4(0.0, scale, 0.0, 0.0),
        vec4(0.0, 0.0, scale, 0.0),
        vec4(flock_buffer[gl_InstanceID].position, 1.0)
    );
    gl_Position = pMat * vMat * mMat * (lmMat * vPos);
}
