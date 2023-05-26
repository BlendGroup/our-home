#version 460 core
layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmmisiveColor;
uniform vec3 color;
uniform bool selected;
void main(void) {
	EmmisiveColor = vec4(0.0);
	if(selected)
		FragColor = vec4(1.0,0.0,1.0,1.0);
	else
		FragColor = vec4(color,1.0);
}
