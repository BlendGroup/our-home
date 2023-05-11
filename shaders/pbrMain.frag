#version 460 core

#define PI 3.141592653589793238462643
const float MAX_REFLECTION_LOD = 4.0; // mips in range [0, 4]
//output
layout(location = 0)out vec4 fragColor;
layout(location = 1)out vec4 emmitColor;

//input
in VS_OUT {
    vec3 P;
	vec3 N;
	vec2 Tex;
	mat3 TBN;
} fs_in;

// structs

struct material_t{
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    vec3 emissive;
    float metallic;
    float roughness;
    float opacity;
};

struct BaseLight{

    vec3 color;
    float intensity;
};

struct DirectionalLight{
    BaseLight base;
    vec3 direction;
};

struct PointLight{
    BaseLight base;
    vec3 position;
    float radius;
};

struct SpotLight{

    PointLight point;
    vec3 direction;
    float inner_angle;
    float outer_angle;
};

struct light_t{
    vec3 diffuse;
    vec3 position;
};

//uniforms

uniform vec3 viewPos;
uniform material_t material;
uniform DirectionalLight dl[2];
uniform PointLight pl[10];
uniform SpotLight sl[5];

uniform int numOfDL;
uniform int numOfPoints;
uniform int numOfSpots;
uniform bool specularGloss;
uniform bool IBL;
uniform bool isTexture;

layout (binding = 0)uniform sampler2D texture_diffuse;
layout (binding = 1)uniform sampler2D texture_normal;
layout (binding = 2)uniform sampler2D texture_PBR;
layout (binding = 3)uniform sampler2D texture_specular;
layout (binding = 4)uniform sampler2D texture_emissive;
layout (binding = 8)uniform samplerCube texture_irradiance;
layout (binding = 9)uniform samplerCube texture_prefilter;
layout (binding = 10)uniform sampler2D texture_brdf_lut;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = normalize(2.0 * texture(texture_normal,fs_in.Tex).rgb - 1.0);
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

    return nom / max(denom,1e-5);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom,1e-5);
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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
} 

float getSquareFalloffAttenuation(vec3 pos_to_light, float light_inv_radius){

    float distance_square = dot(pos_to_light,pos_to_light);
    float factor = distance_square * light_inv_radius * light_inv_radius;
    float smooth_factor = max(1.0 - factor * factor,0.0);
    return (smooth_factor * smooth_factor) / max(distance_square,1e-5);
}

float getSpotAngleAttenuation(vec3 l, vec3 light_dir,float inner_angle,float outer_angle){

    float cos_outer = cos(outer_angle);
    float spot_scale = 1.0 / max(cos(inner_angle) - cos_outer,1e-5);
    float spot_offset = -cos_outer * spot_scale;
    float cd = dot(normalize(-light_dir),l);
    float attenuation = clamp(cd * spot_scale + spot_offset, 0.0,1.0);
    return attenuation;
}

vec3 indirectLightingDiffuse(vec3 N, vec3 P)
{
    vec3 albedo = isTexture ? texture(texture_diffuse,fs_in.Tex).rgb : material.diffuse;
    float metallic = material.metallic;
    if(isTexture)
    {
       metallic = specularGloss ? max(max(texture(texture_specular,fs_in.Tex).r,texture(texture_specular,fs_in.Tex).g),texture(texture_specular,fs_in.Tex).b) : texture(texture_PBR,fs_in.Tex).r;
    }
    float roughness = material.roughness;
    if(isTexture) 
    {
       roughness = specularGloss ? 1.0 - texture(texture_PBR,fs_in.Tex).r : texture(texture_PBR,fs_in.Tex).g;
    }

    float ao = 1.0;
    if(isTexture) 
    {
       ao = specularGloss ? 1.0 - texture(texture_PBR,fs_in.Tex).g : texture(texture_PBR,fs_in.Tex).b;
    }

    N = isTexture ? getNormalFromMap() : N;

    vec3 w0 = normalize(viewPos - P);
    vec3 r = reflect(-w0, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0,albedo,metallic);

    vec3 F = fresnelSchlickRoughness(max(dot(N,w0),0.0),F0,roughness);
    vec3 ks = F;
    vec3 kd = 1.0 - ks;
    kd *= (1.0 - metallic);

    // diffuse IBL term
    vec3 irradiance = texture(texture_irradiance,N).rgb;
    vec3 diffuse = albedo * irradiance;

    // specular IBL term
    vec3 prefilter_color = textureLod(texture_prefilter,r,roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(texture_brdf_lut,vec2(max(dot(N,w0),0.0),roughness)).rg;
    vec3 specular = prefilter_color * (F * brdf.x + brdf.y);

    //total indirect lighting
    return (kd * diffuse + specular) * ao;
}

vec3 pbr(BaseLight base, vec3 direction, vec3 N, vec3 P){

    vec3 albedo = isTexture ? texture(texture_diffuse,fs_in.Tex).rgb : material.diffuse;
    float metallic = material.metallic;
    if(isTexture)
    {
       metallic = specularGloss ? max(max(texture(texture_specular,fs_in.Tex).r,texture(texture_specular,fs_in.Tex).g),texture(texture_specular,fs_in.Tex).b) : texture(texture_PBR,fs_in.Tex).r;
    }
    float roughness = material.roughness;
    if(isTexture) 
    {
       roughness = specularGloss ? 1.0 - texture(texture_PBR,fs_in.Tex).r : texture(texture_PBR,fs_in.Tex).g;
    }

    N = isTexture ? getNormalFromMap() : N;

    vec3 w0 = normalize(viewPos - fs_in.P);
    vec3 wi = normalize(direction);
    vec3 h  = normalize(w0 + wi);
    vec3 radiance = base.color * base.intensity;

    // fresnel reflectance
    vec3 F0 = vec3(0.04);
    F0 = mix(F0,albedo,metallic);
    vec3 F = fresnelSchlick(max(dot(h, w0), 0.0), F0);

    // cook-torrance brdf
    float NDF = DistributionGGX(N, h, roughness);   
    float G   = GeometrySmith(N, w0, wi, roughness);

    float NdotL = max(dot(N,wi),0.0);
    vec3 num = NDF * G * F;

    float denom = 4.0 * max(dot(N,w0),0.0) * NdotL;
    vec3 specular = num / max(denom,1e-5);

    vec3 ks = F;
    vec3 kd = 1.0 - ks;
    kd = kd * (1.0 - metallic);

    return (kd * albedo / PI + specular) * radiance * NdotL;
}

vec3 calcDirectionalLight(DirectionalLight light, vec3 N, vec3 P){
    
    return pbr(light.base, -light.direction, N, P);
}

vec3 calcPointLight(PointLight light, vec3 N, vec3 P){

    vec3 light_direction = light.position - P;
    float attenuation = getSquareFalloffAttenuation(light_direction, 1.0 / max(light.radius,1e-5));
    return pbr(light.base, light_direction, N, P) * attenuation;
}

vec3 calcSpotLight(SpotLight light, vec3 N, vec3 P){

   vec3 l = normalize(P - light.point.position);
   float attenuation = getSpotAngleAttenuation(l,-light.direction,light.inner_angle,light.outer_angle);
    return calcPointLight(light.point, N, P) * attenuation;
}

void main(void) {

    vec3 directional = vec3(0.0);
    for(int i = 0; i < numOfDL; i++){
        directional += calcDirectionalLight(dl[i],normalize(fs_in.N),fs_in.P);
    }

    vec3 point = vec3(0.0);

    for(int i = 0; i < numOfPoints; i++){
        point += calcPointLight(pl[i],normalize(fs_in.N),fs_in.P);
    }

    vec3 spot = vec3(0.0);

    for(int i = 0; i < numOfSpots; i++){
        spot += calcSpotLight(sl[i],normalize(fs_in.N),fs_in.P);
    }

    vec3 ambient = vec3(0.0);
    if(IBL){
        ambient = indirectLightingDiffuse(normalize(fs_in.N),fs_in.P) + texture(texture_emissive,fs_in.Tex).rgb;
    }

    fragColor = vec4(ambient + directional + point + spot,material.opacity);
    emmitColor = vec4(texture(texture_emissive,fs_in.Tex).rgb + material.emissive,material.opacity);
}
