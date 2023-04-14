#version 460 core

uniform int numMeshes;

void main(void) {
	const vec4 vertices[] = vec4[](
		vec4(-0.5, 0.0, -0.5, 1.0),
		vec4(0.5, 0.0, -0.5, 1.0),
		vec4(-0.5, 0.0, 0.5, 1.0),
		vec4(0.5, 0.0, 0.5, 1.0)
	);

	int x = gl_InstanceID / numMeshes;
	int y = gl_InstanceID % numMeshes;
	vec2 offs = vec2(x, y);

	gl_Position = vertices[gl_VertexID] + vec4(float(x - (numMeshes / 2 - 1)), 0.0, float(y - (numMeshes / 2 - 1)), 0.0);
}