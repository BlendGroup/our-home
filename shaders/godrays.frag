#version 460 core

in VS_OUT {
    vec2 uv;
} fs_in;

layout(location = 0) uniform sampler2D texOcclusion;
layout(location = 1) uniform vec2 screenSpaceEmissiveObjectPos;
layout(location = 2) uniform float density;
layout(location = 3) uniform float weight;
layout(location = 4) uniform float decay;
layout(location = 5) uniform float exposure;
layout(location = 6) uniform int samples;

layout(location = 0) out vec4 fragColor;

void main(void) {
    vec2 texCoord = fs_in.uv;
    vec2 deltaTexCoord = texCoord - screenSpaceEmissiveObjectPos;
    deltaTexCoord *= density / float(samples);

    float illuminationDecay = 1.0;
    for(int i = 0; i < samples; i++) {
        texCoord -= deltaTexCoord;
        fragColor += texture(texOcclusion, texCoord) * illuminationDecay * weight;
        illuminationDecay *= decay;
    }

    fragColor *= exposure;
}
