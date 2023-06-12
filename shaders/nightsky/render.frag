#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmmissionColor;

in vec3 starcolor;

void main(void) {
	vec2 ppos = gl_PointCoord.xy * 2.0 - 1.0;
	vec2 abspos = abs(ppos);
	float w = 0.5;
	float h = 0.5;
	float exp1_m = 2.0 / 4.0;
	float exp2_m = -4.0 / 2.0;
	float inner = 1.0 - (w * h * pow(pow(h * abspos.x, exp1_m) + pow(w * abspos.y, exp1_m), exp2_m));
	if(inner > 0.0) {
		discard;
	}
	FragColor = vec4(starcolor, 1.0);
	EmmissionColor = vec4(starcolor, 1.0) * 2.0;
} 