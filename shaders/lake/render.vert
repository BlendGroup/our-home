#version 460 core

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

uniform vec3 cameraPos;

out vec2 texCoord;
out vec4 clipPos;
out vec3 normal;
out vec3 cameraVector;

void main(void) {
	vec3 lakePos[] = vec3[4](
		vec3(1.0, 0.0, 1.0),
		vec3(-1.0, 0.0, 1.0),
		vec3(1.0, 0.0, -1.0),
		vec3(-1.0, 0.0, -1.0)
	);
	vec4 worldPos = mMat * vec4(lakePos[gl_VertexID], 1.0);
	clipPos = pMat * vMat * worldPos;
	texCoord = clamp(lakePos[gl_VertexID].xz, 0.0, 1.0);
	normal = vec3(0.0, 1.0, 0.0);
	cameraVector = cameraPos - worldPos.xyz;
	gl_Position = clipPos;
}