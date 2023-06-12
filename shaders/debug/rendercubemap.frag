#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmmissionColor;
layout(location = 2)out vec4 OcclusionColor;

in vec3 TexCoords;
uniform samplerCube skyboxColor;
uniform samplerCube skyboxEmmission;
void main()
{    
    vec3 color = texture(skyboxColor, TexCoords).rgb; 
    vec3 emmision = texture(skyboxColor, TexCoords).rgb; 
    FragColor = vec4(color, 1.0);
    EmmissionColor = vec4(emmision, 1.0);
    OcclusionColor = vec4(0.0, 0.0, 0.0,1.0);
}