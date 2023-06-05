#version 450 core

layout (quads, fractional_odd_spacing) in;

in TCS_OUT {
	vec2 tc;
} tes_in[];

out TES_OUT {
	vec2 tc;
	vec3 pos;
	vec3 nor;
} tes_out;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;
uniform sampler2D texHeight;
uniform sampler2D texNormal;
uniform float clipy;

void calcTexCoordAndPosition(in vec2 tessCoord, out vec4 P, out vec2 TC) {
	vec2 tc1 = mix(tes_in[0].tc, tes_in[1].tc, tessCoord.x);
	vec2 tc2 = mix(tes_in[2].tc, tes_in[3].tc, tessCoord.x);
	TC = mix(tc2, tc1, tessCoord.y);

	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, tessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, tessCoord.x);
	P = mix(p2, p1, tessCoord.y);
}

void main(void) {
	vec4 P[5];
	vec2 TC[5];

	vec4 Pi;
	vec2 Ti;

	calcTexCoordAndPosition(gl_TessCoord.xy, P[0], TC[0]);
	calcTexCoordAndPosition(gl_TessCoord.xy + vec2(1.0, 0.0), P[1], TC[1]);
	calcTexCoordAndPosition(gl_TessCoord.xy + vec2(-1.0, 0.0), P[2], TC[2]);
	calcTexCoordAndPosition(gl_TessCoord.xy + vec2(0.0, 1.0), P[3], TC[3]);
	calcTexCoordAndPosition(gl_TessCoord.xy + vec2(0.0, -1.0), P[4], TC[4]);

	P[0].y += texture(texHeight, TC[0]).r;
	P[1].y += texture(texHeight, TC[1]).r;
	P[2].y += texture(texHeight, TC[2]).r;
	P[3].y += texture(texHeight, TC[3]).r;
	P[4].y += texture(texHeight, TC[4]).r;

	float x = P[2].y - P[1].y;
	float z = P[3].y - P[4].y;

	tes_out.tc = TC[0];
	// tes_out.nor = mat3(mMat) * texture(texNormal, tc).rgb;
	// tes_out.nor = texture(texNormal, TC[0]).rgb;
	tes_out.nor = normalize(vec3(x, 2.0, z));
	tes_out.pos = vec3(mMat * P[0]);
	gl_Position = pMat * vMat * mMat * P[0];

	vec4 clipingPlaneReflection = vec4(0.0, 1.0, 0.0, -clipy);
	vec4 clipingPlaneRefraction = vec4(0.0, -1.0, 0.0, clipy);

	gl_ClipDistance[0] = dot(mMat * P[0], clipingPlaneReflection);
	gl_ClipDistance[1] = dot(mMat * P[0], clipingPlaneRefraction);
}