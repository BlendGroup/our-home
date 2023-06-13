#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;
layout(location = 3)in vec3 vTangent;
layout(location = 4)in vec3 vBitangent;

uniform int instanceOffset;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;
uniform float clipy;

layout(std140) uniform position_ubo {
	vec4 position[420];
} ubo;

out VS_OUT {
    vec3 P;
	vec3 N;
	vec2 Tex;
	mat3 TBN;
	int treeid;
} vs_out;

void main(void) {
	vec4 P = mMat * vPos + ubo.position[instanceOffset + gl_InstanceID];
	gl_Position = pMat * vMat * P;
    vs_out.P = vec3(P);
	vs_out.N = mat3(mMat) * vNor;
	vs_out.TBN = mat3(mMat) * mat3(vTangent,vBitangent,vNor);
	vs_out.Tex = vTex;
	vs_out.treeid = instanceOffset + gl_InstanceID;

	vec4 clipingPlaneReflection = vec4(0.0, 1.0, 0.0, -clipy);
	vec4 clipingPlaneRefraction = vec4(0.0, -1.0, 0.0, clipy);

	gl_ClipDistance[0] = dot(P, clipingPlaneReflection);
	gl_ClipDistance[1] = dot(P, clipingPlaneRefraction);
}
