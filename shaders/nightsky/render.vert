#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vColor;

out int layerid;
out vec3 color;

void main(void) {
	layerid = gl_VertexID % 6;
	color = vColor;
	gl_PointSize = vPos.z;
	gl_Position = vec4(vPos.xy, 0.0, 1.0);
}