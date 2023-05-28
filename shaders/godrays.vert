#version 460 core

vec2 screenPos[] = vec2[] (
    vec2( 1.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0,-1.0),
    vec2( 1.0,-1.0)
);

out VS_OUT {
    vec2 uv;
} vs_out;

void main(void) {
    vs_out.uv = clamp(screenPos[gl_VertexID], 0.0, 1.0);
    gl_Position = vec4(screenPos[gl_VertexID], 0.0, 1.0);
}
