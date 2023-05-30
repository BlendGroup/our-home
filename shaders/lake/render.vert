#version 460 core

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

out vec4 clipPos;

void main(void) {
	vec3 lakePos[] = vec3[4](
		vec3(1.0, 0.0, 1.0),
		vec3(-1.0, 0.0, 1.0),
		vec3(1.0, 0.0, -1.0),
		vec3(-1.0, 0.0, -1.0)
	);
	clipPos = pMat * vMat * mMat * vec4(lakePos[gl_VertexID], 1.0);
	gl_Position = clipPos;
}