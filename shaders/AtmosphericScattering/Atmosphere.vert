// Atmospheric scattering vertex shader
// Author: Sean O'Neil
// Copyright (c) 2004 Sean O'Neil

#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;
layout(location = 3)in vec3 vTangent;
layout(location = 4)in vec3 vBitangent;

uniform mat4 M;
uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out vec3 fsPosition;

void main(){

    fsPosition = vec3(M * vPos);
    gl_Position = pMat * vMat * mMat * vPos;
}
