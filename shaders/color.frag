#version 460 core

uniform vec4 color;
uniform vec4 emissive;
uniform vec4 occlusion;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 emmisiveColor;
layout(location = 2) out vec4 occlusionColor;

void main(void) {
    fragColor = color;
	emmisiveColor = emissive;
	occlusionColor = occlusion;
}
