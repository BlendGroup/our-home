#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 2)in vec2 vTex;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out vec2 TexCoord;

void main(void) {
    gl_Position = pMat * vMat * mMat * vPos;
    TexCoord = vTex;
}