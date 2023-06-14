#version 460 core

layout(location = 0)in vec3 a_position;
layout(location = 1)in vec2 a_coordinates;

out vec3 v_position;
out vec2 v_coordinates;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

uniform float size;
uniform float geometrySize;

uniform sampler2D displacementMap;

void main (void) {
	vec3 position = a_position + texture(displacementMap, a_coordinates).rgb * (geometrySize / size);

	v_position = position;
	v_coordinates = a_coordinates;

	gl_Position = pMat * vMat * mMat * vec4(position, 1.0);
}
