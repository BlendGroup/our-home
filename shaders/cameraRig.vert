#version 460 core

layout(location = 0) in vec4 vPos;

layout(location = 0) uniform mat4 mMat;
layout(location = 1) uniform mat4 vMat;
layout(location = 2) uniform mat4 pMat;
layout(location = 3) uniform bool isPoint;

void main(void) {
    gl_Position = pMat * vMat * mMat * vPos;
}
