#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;
layout(location = 3)in vec3 vTangent;
layout(location = 4)in vec3 vBitangent;
layout(location = 5)in ivec4 vBoneIds;
layout(location = 6)in vec4 vWeights;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;
uniform mat4 bMat[100];

out VS_OUT {
    vec3 P;
	vec3 N;
	vec2 Tex;
	mat3 TBN;
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
    vs_out.P = vec3(mMat * totalPosition);
	vs_out.N = mat3(mvMat) * totalNormal;
	vs_out.TBN = mat3(mMat) * mat3(vTangent,vBitangent,vNor);
	vs_out.Tex = vTex;
}