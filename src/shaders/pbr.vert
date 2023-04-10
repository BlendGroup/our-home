#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;
layout(location = 3)in vec3 vTangent;
layout(location = 4)in vec3 vBitangent;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out VS_OUT {
    vec3 P;
	vec3 N;
	vec2 Tex;
} vs_out;

void main(void) {
	mat4 mvMat = vMat * mMat;
	vec4 P = mvMat * vPos;
	gl_Position = pMat * P;
    vs_out.P = vec3(mMat*vPos);
	vs_out.N = mat3(mMat) * vNor;
	vs_out.Tex = vTex;
}