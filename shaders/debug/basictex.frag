#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmissionColor;
layout(location = 2)out vec4 OcclusionColor;

uniform sampler2D texture_diffuse;

in VS_OUT {
	vec2 texCoord;
} fs_in;

void main(void) {
	FragColor = texture(texture_diffuse, fs_in.texCoord);
	EmissionColor = vec4(0.0);
	OcclusionColor = vec4(0.0);
}