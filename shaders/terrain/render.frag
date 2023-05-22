#version 460 core

layout(location = 0)out vec4 FragColor;

uniform vec3 cameraPos;

uniform sampler2D texMap;

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
	calcPhongLightInWorld(fs_in.pos, fs_in.nor, vec3(0.0, 800.0, 0.0), cameraPos, 45.0, diffuse, specular);
	FragColor = vec4(texture(texMap, fs_in.tc).rgb * diffuse + vec3(0.1, 0.1, 0.1) * specular, 1.0);
}