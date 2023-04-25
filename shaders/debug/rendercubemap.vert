#version 460 core

out vec3 texCoord;

void main(void) {
	vec4 vPos[] = vec4[4](
		vec4(0.8, 0.8, 0.0, 1.0),
		vec4(-0.8, 0.8, 0.0, 1.0),
		vec4(0.8, -0.8, 0.0, 1.0),
		vec4(-0.8, -0.8, 0.0, 1.0)
	);

	texCoord = vPos[gl_VertexID].xyz;
	gl_Position = vPos[gl_VertexID];
}