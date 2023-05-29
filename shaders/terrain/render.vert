#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec2 vTex;

out VS_OUT {
	vec2 tc;
} vs_out;

void main(void) {
	vs_out.tc = vTex;
	gl_Position = vPos;
}