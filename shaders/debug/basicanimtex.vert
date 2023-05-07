#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 2)in vec2 vTex;
layout(location = 5)in ivec4 vBoneIds;
layout(location = 6)in vec4 vWeights;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;
uniform mat4 bMat[100];

out VS_OUT {
	vec2 texCoord;
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
	}
	mat4 mvMat = vMat * mMat;
	vec4 P = mvMat * totalPosition;
	gl_Position = pMat * P;
    vs_out.texCoord = vTex;
}