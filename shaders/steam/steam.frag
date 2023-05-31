/* Referred from: https://www.shadertoy.com/view/4dS3Wd */

// By Morgan McGuire @morgan3d, http://graphicscodex.com
// Reuse permitted under the BSD license.

// All noise functions are designed for values on integer scale.
// They are tuned to avoid visible periodicity for both positive and
// negative coordinates within a few orders of magnitude.

#version 460 core

// inputs
in VS_OUT {
    vec4 pos;
} fs_in;

// outputs
layout(location = 0)out vec4 fragColor;
layout(location = 1)out vec4 emissiveColor;
layout(location = 2)out vec4 occlusionColor;

// uniforms
uniform float time;

// For multiple octaves
#define NUM_NOISE_OCTAVES 5

// Precision-adjusted variations of https://www.shadertoy.com/view/4djSRW
float hash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.13);
    p3 += dot(p3, p3.yzx + 3.333);
    return fract((p3.x + p3.y) * p3.z);
}

float noise(vec2 x) {
    vec2 i = floor(x);
    vec2 f = fract(x);

	// Four corners in 2D of a tile
	float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    // Simple 2D lerp using smoothstep envelope between the values.
	// return vec3(mix(mix(a, b, smoothstep(0.0, 1.0, f.x)),
	//			mix(c, d, smoothstep(0.0, 1.0, f.x)),
	//			smoothstep(0.0, 1.0, f.y)));

	// Same code, with the clamps in smoothstep and common subexpressions
	// optimized away.
    vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

float fbm(vec2 x) {
	float v = 0.0;
	float a = 0.4;
	vec2 shift = vec2(100);
	// Rotate to reduce axial bias
    mat2 rot = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));
	for (int i = 0; i < NUM_NOISE_OCTAVES; ++i) {
		v += a * noise(x);
		x = rot * x * 2.0 + shift;
		a *= 0.5;
	}
	return v;
}

void main(void) {
    // vec2 coord = gl_FragCoord.xy * 0.0025 - vec2(gl_FragCoord.y / 2.0, time * 0.1);
    vec2 coord = fs_in.pos.xy * 0.8 - vec2(0.0, time);
    float n = fbm(coord);
    n *= n * n;
	fragColor.rgb = pow(n, 0.8) * 1.3 * normalize(vec3(1.0)) + vec3(n * 0.25);
    emissiveColor = vec4(0.0, 0.0, 0.0, 1.0);
    occlusionColor = vec4(0.0, 0.0, 0.0, 1.0);
}
