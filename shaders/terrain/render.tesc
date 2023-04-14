#version 450 core

layout (vertices = 4) out;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;
uniform float maxTess;
uniform float minTess;

void main(void) {
	if(gl_InvocationID == 0) {
		vec4 p0 = pMat * vMat * mMat * gl_in[0].gl_Position;
		vec4 p1 = pMat * vMat * mMat * gl_in[1].gl_Position;
		vec4 p2 = pMat * vMat * mMat * gl_in[2].gl_Position;
		vec4 p3 = pMat * vMat * mMat * gl_in[3].gl_Position;
		p0 /= p0.w;
		p1 /= p1.w;
		p2 /= p2.w;
		p3 /= p3.w;
		float l0 = distance(p2.xy, p0.xy) * (maxTess - minTess) + minTess;
		float l1 = distance(p3.xy, p2.xy) * (maxTess - minTess) + minTess;
		float l2 = distance(p3.xy, p1.xy) * (maxTess - minTess) + minTess;
		float l3 = distance(p1.xy, p0.xy) * (maxTess - minTess) + minTess;
		gl_TessLevelOuter[0] = l0;
		gl_TessLevelOuter[1] = l1;
		gl_TessLevelOuter[2] = l2;
		gl_TessLevelOuter[3] = l3;
		gl_TessLevelInner[0] = min(l1, l3);
		gl_TessLevelInner[1] = min(l0, l2);
	}
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}