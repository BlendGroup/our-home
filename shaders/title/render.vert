#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out vec3 N;

void main(void) {
	gl_Position = pMat * vMat * mMat * vPos;
	N = vNor;
}