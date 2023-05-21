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

void main(void) {
	vec2 tc1 = mix(tes_in[0].tc, tes_in[1].tc, gl_TessCoord.x);
	vec2 tc2 = mix(tes_in[2].tc, tes_in[3].tc, gl_TessCoord.x);
	vec2 tc = mix(tc2, tc1, gl_TessCoord.y);

	vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
	vec4 p = mix(p2, p1, gl_TessCoord.y);

	p.y += texture(texHeight, tc).r * 50.0;

	tes_out.tc = tc;
	tes_out.nor = mat3(mMat) * (texture(texNormal, tc).rgb);
	tes_out.pos = vec3(mMat * p);
	gl_Position = pMat * vMat * mMat * p;
}