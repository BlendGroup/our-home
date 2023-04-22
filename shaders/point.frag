#version 460 core

layout(location = 0)out vec4 FragColor;

uniform vec3 color;

void main(void) {
	FragColor = vec4(normalize(color),1.0);
}