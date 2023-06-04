#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;
layout(location = 3)in vec3 vTangent;
layout(location = 4)in vec3 vBitangent;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out vec3 Normal;

void main(){

    Normal = vNor;
    gl_Position = pMat * vMat * mMat * vPos;
}