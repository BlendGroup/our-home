#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmissionColor;
layout(location = 2)out vec4 OcclusionColor;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_emmission;
uniform sampler2D texture_occlusion;

in VS_OUT {
	vec2 texCoord;
} fs_in;

void main(void) {
	FragColor = texture(texture_diffuse, fs_in.texCoord);
	EmissionColor = texture(texture_emmission, fs_in.texCoord);
	OcclusionColor = texture(texture_occlusion, fs_in.texCoord);
}