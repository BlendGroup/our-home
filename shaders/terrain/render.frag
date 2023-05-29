#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmissionColor;

uniform vec3 cameraPos;
uniform float texScale;

uniform sampler2D texMap;
uniform sampler2D texDiffuseGrass;
uniform sampler2D texDiffuseDirt;
uniform sampler2D texDiffuseMountain;
uniform sampler2D texLake;

in TES_OUT {
	vec2 tc;
	vec3 pos;
	vec3 nor;
} fs_in;

void calcPhongLightInWorld(in vec3 posInWorld, in vec3 norInWorld, in vec3 lightPos, in vec3 viewPos, in float shininess, out float diffuse, out float specular) {
	vec3 N = normalize(norInWorld);
	vec3 L = normalize(lightPos - posInWorld);
	vec3 V = normalize(viewPos - posInWorld);
	vec3 R = reflect(-L, N);
	diffuse = max(dot(N, L), 0.0);
	specular = pow(max(dot(V, R), 0.0), shininess);
}

void main(void) {
	float diffuse, specular;
	calcPhongLightInWorld(fs_in.pos, fs_in.nor, vec3(0.0, 10000.0, 0.0), cameraPos, 45.0, diffuse, specular);
	float mixVal = texture(texMap, fs_in.tc).r;
	vec3 difColor = mix(texture(texDiffuseGrass, fs_in.tc * texScale).rgb, texture(texDiffuseMountain, fs_in.tc * texScale).rgb, mixVal);
	FragColor = vec4(diffuse * difColor + vec3(0.1, 0.1, 0.1) * specular, 1.0);
	EmissionColor = vec4(0.0);
}