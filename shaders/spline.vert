#version 460 core

layout(location = 0) in vec4 vPos;

layout(location = 0) uniform mat4 mvpMatrix; 
layout(location = 1) uniform vec4 color;
layout(location = 2) uniform bool isPoint;

out VS_OUT {
    vec4 color;
} vs_out;

const vec4 pointColors[] = {
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0),
    vec4(1.0, 0.0, 0.0, 1.0),

    vec4(0.0, 0.0, 1.0, 1.0),
    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0), 

    vec4(1.0, 0.0, 0.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(1.0, 0.0, 0.0, 1.0),

    vec4(0.0, 0.0, 1.0, 1.0),
    vec4(0.0, 1.0, 0.0, 1.0),
    vec4(0.0, 0.0, 1.0, 1.0) 
};

void main(void) {
    gl_Position = mvpMatrix * vPos;
    if(isPoint) {
        vs_out.color = pointColors[gl_VertexID % 12];
        return;
    }
    vs_out.color = color;
}