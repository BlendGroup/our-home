#version 460 core

#define MAX_BOIDS_PER_FLOCK 1024

struct boid_t {
    vec4 position;
    vec4 velocity;
};
layout(std140, binding = 0) uniform FlockBlock {
    boid_t flock_buffer[MAX_BOIDS_PER_FLOCK];
};

uniform mat4 vMat;
uniform mat4 pMat;
uniform float scale;

const vec4 quadVerts[4] = vec4[] (
    vec4(-1.0,-1.0, 0.0, 1.0),
    vec4( 1.0,-1.0, 0.0, 1.0),
    vec4(-1.0, 1.0, 0.0, 1.0),
    vec4( 1.0, 1.0, 0.0, 1.0)
);

void main(void) {
    mat4 mMat = mat4(
        vec4(scale, 0.0, 0.0, 0.0),
        vec4(0.0, scale, 0.0, 0.0),
        vec4(0.0, 0.0, scale, 0.0),
        flock_buffer[gl_InstanceID].position
    );
    mat4 mvMat = vMat * mMat;
    // TODO: billboard correctly
    // mvMat[0][0] = 1.0;
    // mvMat[1][1] = 1.0;
    // mvMat[2][2] = 1.0;
    gl_Position = pMat * mvMat * quadVerts[gl_VertexID];
}
