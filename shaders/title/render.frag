#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmissionColor;
layout(location = 2)out vec4 OcclusionColor;

uniform sampler2D texDiffuse;
uniform vec3 L;
uniform vec3 lightDirection = vec3(0.0, 0.0, -1.0);
uniform vec2 lightCutOff;
uniform int occlusion;

in vec3 N;
in vec3 P;
in vec2 texCoord;

void main(void) {
	vec3 lightDir = normalize(L - P);  
	float theta = dot(lightDir, normalize(-lightDirection));
	float innerCutOff = cos(radians(lightCutOff.x));
	float outerCutOff = cos(radians(lightCutOff.y));
	float intensity = clamp((theta - outerCutOff) / (innerCutOff - outerCutOff), 0.0, 1.0);
	
	vec3 norm = normalize(N);
	
	float ambient ;
	float diffuse = intensity * max(dot(norm, lightDir), 0.0); 
	vec3 color = texture(texDiffuse, texCoord).rgb;

	FragColor = vec4((diffuse + ambient) * color, 1.0);
	EmissionColor = vec4(0.0);
	OcclusionColor = vec4(diffuse * color, 1.0) * occlusion;
}