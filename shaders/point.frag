#version 460 core

layout(location = 0)out vec4 FragColor;

uniform sampler2D texSampler;

void main(void) {
	FragColor = vec4(vec3(texture(texSampler, gl_PointCoord.xy).r * 0.5 + 0.5), 1.0);
}