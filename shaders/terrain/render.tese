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
uniform sampler2D texHeight1;
uniform sampler2D texNormal1;
uniform sampler2D texHeight2;
uniform sampler2D texNormal2;
uniform sampler2D texMap;
uniform sampler2D texLake;
uniform float amplitudeMin;
uniform float amplitudeMax;
uniform float lakeDepth;

void main(void) {
	vec2 tc1 = mix(tes_in[0].tc, tes_in[1].tc, gl_TessCoord.x);
	vec2 tc2 = mix(tes_in[2].tc, tes_in[3].tc, gl_TessCoord.x);
	vec2 tc = mix(tc2, tc1, gl_TessCoord.y);

	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
	vec4 p = mix(p2, p1, gl_TessCoord.y);

	p.y += mix(texture(texHeight1, tc).r, texture(texHeight2, tc).r, texture(texMap, tc).r) * mix(amplitudeMin, amplitudeMax, texture(texMap, tc).r) - texture(texLake, tc).r * lakeDepth;

	tes_out.tc = tc;
	tes_out.nor = mat3(mMat) * mix(texture(texNormal1, tc).rgb, texture(texNormal2, tc).rgb, texture(texMap, tc).r);
	tes_out.pos = vec3(mMat * p);
	gl_Position = pMat * vMat * mMat * p;
}