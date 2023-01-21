#version 410 core

layout(location = 0)out vec4 FragColor;

uniform sampler2D texSampler;

void main(void) {
	FragColor = texture(texSampler, gl_PointCoord.xy);
}