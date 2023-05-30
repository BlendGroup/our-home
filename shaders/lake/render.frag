#version 460 core

in vec4 clipPos;

uniform sampler2D texRefraction;
uniform sampler2D texReflection;

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmissionColor;

void main(void) {
	vec2 refractionCoords = (clipPos.xy / clipPos.w) * 0.5 + 0.5;
	vec2 reflectionCoords = vec2(refractionCoords.x, -refractionCoords.y);
	FragColor = mix(texture(texRefraction, refractionCoords), texture(texReflection, reflectionCoords), 0.5) * vec4(0.0, 1.0, 1.0, 0.0);
	EmissionColor = vec4(0.0, 0.0, 0.0, 1.0);
}