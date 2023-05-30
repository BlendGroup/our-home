#version 460 core

in vec4 clipPos;
in vec2 texCoord;
in vec3 normal;
in vec3 cameraVector;

uniform sampler2D texRefraction;
uniform sampler2D texReflection;
uniform sampler2D texDuDv;
uniform float time;

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmissionColor;

void main(void) {
	vec2 refractionCoords = (clipPos.xy / clipPos.w) * 0.5 + 0.5;
	vec2 reflectionCoords = vec2(refractionCoords.x, -refractionCoords.y);

	vec2 distortion1 = (texture(texDuDv, texCoord * 3.0 + vec2(time, -time)).rg * 2.0 - 1.0) * 0.015;
	vec2 distortion2 = (texture(texDuDv, texCoord * 3.0 + vec2(-time, time)).rg * 2.0 - 1.0) * 0.015;

	refractionCoords += distortion1 + distortion2;
	reflectionCoords += distortion1 + distortion2;
	refractionCoords = clamp(refractionCoords, 0.001, 0.999);
	reflectionCoords = vec2(clamp(reflectionCoords.x, 0.001, 0.999), clamp(reflectionCoords.y, -0.999, -0.001));

	vec3 V = normalize(cameraVector);
	vec3 N = normalize(normal);
	float fresnelEffect = pow(dot(V, N), 2.0);

	FragColor = mix(mix(texture(texReflection, reflectionCoords), texture(texRefraction, refractionCoords), fresnelEffect), vec4(0.0, 0.2, 0.4, 1.0), 0.2);
	EmissionColor = vec4(0.0, 0.0, 0.0, 1.0);
}