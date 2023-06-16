#version 460 core

layout(location = 0)out vec4 FragColor;

uniform sampler2D tex;
uniform float fade;

in vec2 texCoord;

void main(void) {
	FragColor = texture(tex, texCoord) * fade;
}