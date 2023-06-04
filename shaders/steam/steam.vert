#version 460 core

layout(location = 0)in vec4 vPos;

out VS_OUT {
    vec4 pos;
} vs_out;

uniform mat4 mvpMatrix;

void main(void) {
    gl_Position = mvpMatrix * vPos;
    vs_out.pos = vPos;
}
