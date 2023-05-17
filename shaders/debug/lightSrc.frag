#version 460 core
layout(location = 0)out vec4 FragColor;
uniform vec3 color;
uniform bool selected;
void main(void) {
	
	if(selected)
		FragColor = vec4(1.0,0.0,1.0,1.0);
	else
		FragColor = vec4(color,1.0);
}
