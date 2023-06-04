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

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

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
	vec3 grassColor = mix(texture(texDiffuseGrass, fs_in.tc * texScale).rgb, texture(texDiffuseDirt, fs_in.tc * texScale).rgb, 1.0 - (noise(fs_in.tc * texScale) * 0.5 + 0.5));
	vec3 difColor = mix(grassColor, texture(texDiffuseMountain, fs_in.tc * texScale).rgb, mixVal);
	FragColor = vec4(diffuse * difColor + vec3(0.1, 0.1, 0.1) * specular, 1.0);
	EmissionColor = vec4(0.0);
}