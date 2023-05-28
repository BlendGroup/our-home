#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmmisionColor;
layout(location = 2)out vec4 OcclusionColor;

uniform sampler2D texSampler;
uniform float alpha;

in vec2 texCoord;

void main(void) {
	FragColor = vec4(texture(texSampler, texCoord).rgb, clamp(alpha, 0.0, 1.0));
	EmmisionColor = vec4(0.0);
	OcclusionColor = vec4(0.0);
}