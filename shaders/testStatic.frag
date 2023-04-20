#version 460 core

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

layout(location = 0)out vec4 fragColor;

in VS_OUT {
	vec3 N;
	vec3 L;
	vec3 V;
	vec2 Tex;
} fs_in;

void main(void) {
	vec3 N = normalize(fs_in.N);
	vec3 L = normalize(fs_in.L);
	vec3 V = normalize(fs_in.V);
	vec3 R = reflect(-L, N);
	float ambient = 0.03;
	float diffuse = max(dot(N, L), 0.0);
	float specular = pow(max(dot(R, V), 0.0), 100.0);
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	vec4 diffuseTextureColor = texture(diffuseTexture, fs_in.Tex);
	vec4 specularTextureColor = texture(specularTexture, fs_in.Tex);
	fragColor = vec4(ambient * lightColor + diffuse * lightColor, 1.0) * diffuseTextureColor + vec4(specular * lightColor, 1.0) * specularTextureColor;
}