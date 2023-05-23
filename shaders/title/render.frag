#version 460 core

layout(location = 0)out vec4 FragColor;

in vec3 N;

void main(void) {
	FragColor = vec4(abs(N), 1.0);
}