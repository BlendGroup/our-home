#version 460 core

uniform mat4 mMat;
uniform mat4 vMat;
uniform mat4 pMat;


void main(void) {
	const vec3 vPos[] = vec3[4](
		vec3(-0.5, 0.0, -0.5),
		vec3(0.5, 0.0, -0.5),
		vec3(-0.5, 0.0, 0.5),
		vec3(0.5, 0.0, 0.5)
	);

	gl_Position = pMat * vMat * mMat * vec4(vPos[gl_VertexID], 1.0);
}