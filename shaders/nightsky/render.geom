#version 460 core

layout(points) in;
layout(points, max_vertices = 4 * 6) out;

in int layerid[];
in vec3 color[];

out vec3 starcolor;

void main(void) {
	gl_Layer = layerid[0];
	starcolor = color[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	EndPrimitive();
}