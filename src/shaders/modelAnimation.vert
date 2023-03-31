#version 460 core 

layout(location = 0) in vec3 a_position; 
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;
layout(location = 3) in vec3 a_tangent;
layout(location = 4) in vec3 a_bitangent;
layout(location = 5) in ivec4 a_boneIds;
layout(location = 6) in vec4 a_weights;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

const int MAX_BONES = 100;
const int MAX_BONES_INFLUENCE = 4;
uniform mat4 gBones[MAX_BONES];

out vec2 out_texcoord;

void main(void) 
{ 
    vec4 totalPosition = vec4(0.0f);

    for(int i = 0; i < MAX_BONES_INFLUENCE; i++)
    {
        if(a_boneIds[i] == -1)
            continue;
        if(a_boneIds[i] >= MAX_BONES)
        {
            totalPosition = vec4(a_position,1.0f);
            break;
        }

        vec4 localPosition = gBones[a_boneIds[i]] * vec4(a_position,1.0f);
        totalPosition += localPosition * a_weights[i];
    }
    //gl_Position = u_Projection * u_View * u_Model * vec4(a_position,1.0f);
    gl_Position = u_Projection * u_View * u_Model * totalPosition;
    out_texcoord = a_texcoord;
}
