#version 460 core 

in vec2 out_texcoord;

uniform sampler2D texture_diffuse1;

out vec4 FragColor;

void main(void) 
{
    //FragColor = texture(texture_diffuse1, out_texcoord);
    FragColor = vec4(1.0f,1.0f,1.0f,1.0f);
}
