#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out VS_OUT {
	vec3 N;
	vec3 L;
	vec3 V;
	vec2 Tex;
} vs_out;

void main(void) {
	mat4 mvMat = vMat * mMat;
	vec4 P = mvMat * vPos;
	gl_Position = pMat * P;
	vs_out.N = mat3(mvMat) * vNor;
	vs_out.L = vec3(100.0, 100.0, 100.0) - P.xyz;
	vs_out.V = -P.xyz;
	vs_out.Tex = vTex;
}