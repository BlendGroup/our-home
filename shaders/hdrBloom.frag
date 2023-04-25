#version 460 core
out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D hdrTex;
uniform sampler2D bloomTex;
uniform float exposure;
uniform float fade;

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
mat3 ACESInputMat = {
    {0.59719, 0.07600, 0.02840},
    {0.35458, 0.90834, 0.13383},
    {0.04823, 0.01566, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
mat3 ACESOutputMat =
{
    { 1.60475, -0.10208, -0.00327},
    {-0.53108,  1.10813, -0.07276},
    {-0.07367, -0.00605,  1.07602 }
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

void main()
{
    vec3 x = (texture(hdrTex,texCoord).rgb + texture(bloomTex,texCoord).rgb) * exposure;

    vec3 color = ACESInputMat * x;
    color = RRTAndODTFit(color);
    color = ACESOutputMat * color;
    vec3 result = vec3(1.0) - exp(-color);
    result = mix(vec3(0.0), result, fade);
    FragColor = vec4(result, 1.0);
}
