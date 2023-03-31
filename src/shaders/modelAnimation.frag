#version 460 core 

in vec2 out_texcoord;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

out vec4 FragColor;

void main(void) 
{
    vec3 color = texture(texture_diffuse1, out_texcoord).rgb * texture(texture_specular1,out_texcoord).rgb;
    FragColor = vec4(color,1.0f);
}
