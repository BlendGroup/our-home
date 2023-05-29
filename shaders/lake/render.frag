#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmissionColor;

void main(void) {
	FragColor = vec4(0.0, 0.3, 1.0, 1.0);
	EmissionColor = vec4(0.0, 0.0, 0.0, 1.0);
}