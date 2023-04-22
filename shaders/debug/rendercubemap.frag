#version 460 core

layout(location = 0)out vec4 FragColor;

uniform samplerCube cubemap;

in vec3 texCoord;

void main(void) {
	FragColor = texture(cubemap, texCoord);
}