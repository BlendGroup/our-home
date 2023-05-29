#version 460 core

layout(location = 0)in vec4 vPos;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

void main(void) {
	gl_Position = pMat * vMat * mMat * vPos;
}