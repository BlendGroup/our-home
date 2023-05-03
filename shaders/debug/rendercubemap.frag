#version 460 core
out vec4 FragColor;
in vec3 TexCoords;
uniform samplerCube skybox;
void main()
{    
    vec3 color = texture(skybox, TexCoords).rgb; 
    FragColor = vec4(color,1.0);
}