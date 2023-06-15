#version 460 core

in vec2 v_coordinates;
in vec3 v_position;

uniform sampler2D normalMap;

uniform vec3 cameraPosition;

uniform vec3 oceanColor;
uniform vec3 skyColor;

uniform vec3 sunDirection;

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmmissionColor;
layout(location = 2)out vec4 OcclusionColor;

void main (void) {
	vec3 normal = texture(normalMap, v_coordinates).rgb;

	vec3 view = normalize(cameraPosition - v_position);
	float fresnel = 0.02 + 0.98 * pow(1.0 - dot(normal, view), 5.0);
	vec3 sky = fresnel * skyColor;

	float diffuse = clamp(dot(normal, normalize(sunDirection)), 0.0, 1.0);
	vec3 water = (1.0 - fresnel) * oceanColor * diffuse;

	vec3 color = sky + water;

	FragColor = vec4(color, 1.0);
	EmmissionColor = vec4(0.0, 0.0, 0.0, 1.0);
	OcclusionColor = vec4(0.0, 0.0, 0.0, 1.0);
}
