#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmissionColor;
layout(location = 2)out vec4 OcclusionColor;

in vec3 N;

void main(void) {
	FragColor = vec4(abs(N), 1.0);
	EmissionColor = vec4(0.0);
	OcclusionColor = vec4(0.0);
}