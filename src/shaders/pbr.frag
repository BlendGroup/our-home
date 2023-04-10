#version 460 core

const float PI = 3.14159265359;

layout(location = 0)out vec4 fragColor;

struct material_t{
    vec3 diffuse;
    float metallic;
    float roughness;
    float ao;
};

struct light_t{
    vec3 diffuse;
    vec3 position;
};

in VS_OUT {
    vec3 P;
	vec3 N;
	vec2 Tex;
	mat3 TBN;
} fs_in;

uniform vec3 viewPos;

uniform material_t material;
uniform light_t light[15];
uniform int numOfLights;

uniform bool isTextured;

// Texture Uniforms
uniform sampler2D diffueMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = normalize(2.0 * texture(normalMap,fs_in.Tex).rgb - 1.0);
    return normalize(fs_in.TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main(void) {

    vec3 diffuse;
    float metalness;
    float roughness;
    float ao;
    vec3 N;
    if(isTextured)
    {
        diffuse = texture(diffueMap,fs_in.Tex).rgb * material.diffuse;
        metalness = texture(metallicMap,fs_in.Tex).r * material.metallic;
        roughness = texture(roughnessMap,fs_in.Tex).r * material.roughness;
        ao = texture(aoMap,fs_in.Tex).r * material.ao;
        N = getNormalFromMap();
    }
    else
    {
        diffuse = material.diffuse;
        metalness = material.metallic;
        roughness = material.roughness;
        ao = material.ao;
        N = normalize(fs_in.N);
    }

    vec3 V = normalize(viewPos - fs_in.P);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0,diffuse,metalness);

    vec3 Lo = vec3(0.0);

    for(int i = 0; i < numOfLights; i++)
    {
        vec3 L = normalize(light[i].position - fs_in.P);
        vec3 H = normalize(V+L);
        float dist = length(light[i].position - fs_in.P);
        float attenuation = 1.0 / (dist * dist);

        vec3 radiance = light[i].diffuse * attenuation;

        // Cook Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);   
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N,V),0.0) * max(dot(N,L),0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metalness;

        float NdotL = max(dot(N,L),0.0);
        Lo += (kD * diffuse / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * diffuse * ao;
    fragColor = vec4(ambient+Lo,1.0);
}