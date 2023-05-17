#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 pMat;
uniform mat4 vMat;

void main()
{
    TexCoords = aPos;
    TexCoords.y = -TexCoords.y;
    TexCoords.z = -TexCoords.z;
    mat4 rotView = mat4(mat3(vMat));
    vec4 pos = pMat * rotView * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}