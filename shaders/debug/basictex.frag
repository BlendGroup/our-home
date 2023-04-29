#version 460 core

layout(location = 0)out vec4 FragColor;

uniform sampler2D texture_diffuse;

in VS_OUT {
	vec2 texCoord;
} fs_in;

void main(void) {
	FragColor = texture(texture_diffuse, fs_in.texCoord);
}