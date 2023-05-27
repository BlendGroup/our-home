#version 460 core

vec2 screenPos[] = vec2[] (
    vec2( 1.0f, 1.0f),
    vec2(-1.0f, 1.0f),
    vec2(-1.0f,-1.0f),
    vec2( 1.0f,-1.0f)
);

out VS_OUT {
    vec2 uv;
} vs_out;

void main(void) {
    vs_out.uv = clamp(screenPos[gl_VertexID], 0.0f, 1.0f);
    gl_Position = vec4(screenPos[gl_VertexID], 0.0, 1.0);
}
