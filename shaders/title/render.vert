#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out vec3 N;
out vec3 P;
out vec2 texCoord;

void main(void) {
	gl_Position = pMat * vMat * mMat * vPos;
	N = mat3(mMat) * vNor;
	P = vec3(mMat * vPos);
	texCoord = vec2(vPos);
}