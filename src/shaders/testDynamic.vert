#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;
layout(location = 5)in ivec4 vBoneIds;
layout(location = 6)in vec4 vWeights;

layout(location = 0)uniform mat4 pMat;
layout(location = 1)uniform mat4 vMat;
layout(location = 2)uniform mat4 mMat;
layout(location = 3)uniform mat4 bMat[100];

out VS_OUT {
	vec3 N;
	vec3 L;
	vec3 V;
	vec2 Tex;
} vs_out;

void main(void) {
	vec4 totalPosition = vec4(0.0);
	vec3 totalNormal = vec3(0.0);
	for(int i = 0 ; i < 4; i++) {
		if(vBoneIds[i] == -1) {
			continue;
		}
		vec4 localPosition = bMat[vBoneIds[i]] * vPos;
		totalPosition += localPosition * vWeights[i];
		vec3 localNormal = mat3(bMat[vBoneIds[i]]) * vNor;
		totalNormal += localNormal;
	}
	mat4 mvMat = vMat * mMat;
	vec4 P = mvMat * totalPosition;
	gl_Position = pMat * P;
	vs_out.N = mat3(mvMat) * totalNormal;
	vs_out.L = vec3(100.0, 100.0, 100.0) - P.xyz;
	vs_out.V = -P.xyz;
	vs_out.Tex = vTex;
}