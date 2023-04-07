#version 460 core

layout(location = 0) in vec4 vPos;

layout(location = 0) uniform mat4 mMat;
layout(location = 1) uniform mat4 vMat;
layout(location = 2) uniform mat4 pMat;
layout(location = 3) uniform bool isPlane;

const vec4 faceColors[] = {
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

const vec4 planeColors[] = {
    vec4(0.5, 0.5, 0.5, 1.0),
    vec4(0.5, 0.5, 0.5, 1.0),

    vec4(1.0, 1.0, 1.0, 1.0),
    vec4(1.0, 1.0, 1.0, 1.0)
};

out VS_OUT {
    vec4 color;
} vs_out;

void main(void) {
    gl_Position = pMat * vMat * mMat * vPos;

    if(!isPlane)
        vs_out.color = faceColors[gl_VertexID % 12];
    else
        vs_out.color = planeColors[gl_VertexID % 4];
}
