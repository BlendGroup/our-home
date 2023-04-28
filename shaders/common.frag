#version 460 core

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
    float specular_intensity;
    float specular_power;
    float opacity;
};

struct BaseLight
{
    vec3 color;
    float intensity;
};

struct DirectionalLight
{
    BaseLight base;
    vec3 direction;
};

struct PointLight
{
    BaseLight base;
    vec3 position;
    float radius;
};

struct SpotLight
{
    PointLight point;
    vec3 direction;
    float inner_angle;
    float outer_angle;
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

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_emissive;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = normalize(2.0 * texture(texture_normal,fs_in.Tex).rgb - 1.0);
    return normalize(fs_in.TBN * tangentNormal);
}

vec3 blinnPhong(BaseLight base, vec3 direction, vec3 N, vec3 P)
{
    N = getNormalFromMap();
    float diffuse = max(dot(N, -direction), 0.0f);

    vec3 dir_to_eye  = normalize(viewPos - P);
    vec3 half_vector = normalize(dir_to_eye - direction);
    float specular   = pow(max(dot(half_vector, N), 0.0f), material.specular_power);

    vec3 diffuse_color  = base.color * base.intensity * diffuse * texture(texture_diffuse, fs_in.Tex).rgb + material.diffuse;
    vec3 specular_color = material.specular * specular * material.specular_intensity;
    return diffuse_color + specular_color;
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

vec3 calcDirectionalLight(DirectionalLight light, vec3 normal, vec3 world_pos)
{
    return blinnPhong(light.base, light.direction, normal, world_pos);
}

vec3 calcPointLight(PointLight light, vec3 N, vec3 P)
{
    vec3 light_direction    = light.position - P;
    float attenuation = getSquareFalloffAttenuation(light_direction, 1.0 / max(light.radius,1e-5));
    return blinnPhong(light.base, normalize(light_direction), N, P) * attenuation;
}

vec3 calcSpotLight(SpotLight light, vec3 N, vec3 P)
{

    vec3 l = normalize(P - light.point.position);
    float attenuation = getSpotAngleAttenuation(l,-light.direction,light.inner_angle,light.outer_angle);
    return calcPointLight(light.point,N,P) * attenuation;
}

void main(void) {

    vec3 color = vec3(0.0);

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

    fragColor = vec4(directional + point + spot,material.opacity);
    emmitColor = vec4(texture(texture_emissive,fs_in.Tex).rgb + material.emissive,material.opacity);
}