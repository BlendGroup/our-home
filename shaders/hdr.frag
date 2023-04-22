#version 460 core
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D hdrTex;
uniform float exposure;
uniform float fade;

void main()
{
    vec3 color = texture(hdrTex,texCoord).rgb;
    vec3 result = vec3(1.0) - exp(-color * exposure);
    result = mix(vec3(0.0), result, fade);
    FragColor = vec4(result, 1.0);
}
