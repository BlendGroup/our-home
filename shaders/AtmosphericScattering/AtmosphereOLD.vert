// Atmospheric scattering vertex shader
// Author: Sean O'Neil
// Copyright (c) 2004 Sean O'Neil

#version 460 core

layout (location = 0) in vec4 aPos;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out vec4 vPosition;

void main(){

    vPosition = aPos;
    gl_Position = pMat * vMat * mMat * aPos;
}
