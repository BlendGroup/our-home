#version 460 core

layout(location = 0)in vec3 vPos;

uniform vec3 trans;

void main(void) {
	gl_Position = vec4(vPos + trans, 1.0);
}