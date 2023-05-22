#version 460 core

uniform vec4 color;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 emmisiveColor;

void main(void) {
    fragColor = color;
	emmisiveColor = vec4(0.0);
}
