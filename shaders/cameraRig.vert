#version 460 core

layout(location = 0) in vec4 vPos;

layout(location = 0) uniform mat4 mMat;
layout(location = 1) uniform mat4 vMat;
layout(location = 2) uniform mat4 pMat;
layout(location = 3) uniform bool isPoint;

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

const vec4 lineColor = vec4(1.0, 0.0, 0.0, 1.0);

void main(void) {
    gl_Position = pMat * vMat * mMat * vPos;
    if(isPoint)
        vs_out.color = pointColors[gl_VertexID % 12];
    else
        vs_out.color = lineColor;
}
