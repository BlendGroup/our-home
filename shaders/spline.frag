#version 460 core

layout(location = 1) uniform vec4 color;

out vec4 fragColor;

void main(void) {
    fragColor = color;
}
