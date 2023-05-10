#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 2)in vec2 vTex;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out VS_OUT {
	vec2 texCoord;
} vs_out;

void main(void) {
	vs_out.texCoord = vTex;
	gl_Position = pMat * vMat * mMat * vPos;
}