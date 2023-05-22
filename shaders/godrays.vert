#version 460 core

layout(location = 0) in vec2 pos;

out VS_OUT {
    vec2 uv;
} vs_out;

void main(void) {
    vs_out.uv = pos;
}
