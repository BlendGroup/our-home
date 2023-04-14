#version 450 core

layout (quads, fractional_odd_spacing) in;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

void main(void) {
	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
	vec4 p = mix(p2, p1, gl_TessCoord.y);

	gl_Position = pMat * vMat * mMat * p;
}