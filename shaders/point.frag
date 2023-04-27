#version 460 core

layout(location = 0)out vec4 FragColor;

uniform vec3 color;
uniform samplerCube envmap;
void main(void) {
	FragColor = vec4(normalize(color)+texture(envmap,color).rgb,1.0);
}