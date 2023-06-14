#version 460 core

layout(location = 0)in vec3 a_position;
layout(location = 1)in vec2 a_coordinates;

out vec3 v_position;
out vec2 v_coordinates;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

uniform float u_size;
uniform float u_geometrySize;

uniform sampler2D u_displacementMap;

void main (void) {
	vec3 position = a_position + texture(u_displacementMap, a_coordinates).rgb * (u_geometrySize / u_size);

	v_position = position;
	v_coordinates = a_coordinates;

	gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * vec4(position, 1.0);
}
