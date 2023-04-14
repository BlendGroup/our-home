#version 460 core

layout(location = 0)out vec4 FragColor;

uniform sampler2D texColor;

in TES_OUT {
	vec2 tc;
} fs_in;

void main(void) {	
	FragColor = texture(texColor, fs_in.tc);
	// FragColor = vec4(fs_in.tc, 1.0, 1.0);
}