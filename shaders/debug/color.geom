#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 mvpMat[6];

void main(void) {
	for(int i = 0; i < 6; i++) {
		gl_Layer = i;
		for(int j = 0; j < 3; j++) {
			gl_Position = mvpMat[i] * gl_in[j].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}