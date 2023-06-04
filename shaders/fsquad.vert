#version 460 core

vec2 quad[4] = vec2[] (
    vec2(-1.0,-1.0),
    vec2( 1.0,-1.0),
    vec2(-1.0, 1.0),
    vec2( 1.0, 1.0)
);

out vec2 texCoord;

void main(void) {
	gl_Position = vec4(quad[gl_VertexID], 0.0, 1.0);
    texCoord = clamp(quad[gl_VertexID],0.0f,1.0f);
}