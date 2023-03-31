#version 460 core 

layout(location = 0) in vec3 a_position; 
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;
layout(location = 3) in vec3 a_tangent;
layout(location = 4) in vec3 a_bitangent;

uniform bool hasAnimation;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;
out vec2 out_texcoord;

void main(void) 
{ 
    gl_Position = u_Projection * u_View * u_Model * vec4(a_position,1.0f);
    out_texcoord = a_texcoord;
}
