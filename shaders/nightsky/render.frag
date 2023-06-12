#version 460 core

layout(location = 0)out vec4 FragColor;

in vec3 starcolor;

void main(void) {
	FragColor = vec4(starcolor, 1.0);
}