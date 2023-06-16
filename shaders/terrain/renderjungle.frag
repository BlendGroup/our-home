#version 460 core

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmissionColor;

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


uniform int numOfDL;
uniform int numOfPoints;
uniform int numOfSpots;
uniform DirectionalLight dl[2];
uniform PointLight pl[10];
uniform SpotLight sl[5];

uniform vec3 cameraPos;
uniform float texScale;

uniform sampler2D texDiffuseGrass;
uniform sampler2D texDiffuseDirt;

uniform vec3 ambientColor;

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define OCTAVES 6
float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < OCTAVES; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

in TES_OUT {
	vec2 tc;
	vec3 pos;
	vec3 nor;
} fs_in;


vec2 blinnPhong(vec3 direction, vec3 N, vec3 P)
{
    float diffuse = max(dot(N, -direction), 0.0);

    vec3 dir_to_eye  = normalize(cameraPos - P);
    vec3 half_vector = normalize(dir_to_eye - direction);
    float specular   = pow(max(dot(half_vector, N), 0.0), 45.0);
    return vec2(diffuse,specular);
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

float calcDirectionalLight(DirectionalLight light, vec3 normal, vec3 world_pos, out vec3 diffuse) {
    vec2 op = blinnPhong(light.direction, normal, world_pos);
	diffuse = op.x * light.base.color;
	return op.y;
}

float calcPointLight(PointLight light, vec3 N, vec3 P, out vec3 diffuse){
       vec3 light_direction    = P - light.position;
    float attenuation = getSquareFalloffAttenuation(light_direction, 1.0 / max(light.radius,1e-5));
    vec2 op = blinnPhong(normalize(light_direction), N, P) * attenuation;
	diffuse = op.x * light.base.color * light.base.intensity;
	return op.y;
}

float calcSpotLight(SpotLight light, vec3 N, vec3 P, out vec3 diffuse){
    vec3 l = normalize(P - light.point.position);
    float attenuation = getSpotAngleAttenuation(l,-light.direction,light.inner_angle,light.outer_angle);
    float op = calcPointLight(light.point,N,P,diffuse) * attenuation;
	return op;
}

void main(void) {
    
    float specular = 0.0;
	vec3 diffuse = vec3(0.0);
    for(int i = 0; i < numOfDL; i++){
        specular += calcDirectionalLight(dl[i],normalize(fs_in.nor),fs_in.pos, diffuse);
    }
    for(int i = 0; i < numOfPoints; i++){
        specular += calcPointLight(pl[i],normalize(fs_in.nor),fs_in.pos, diffuse);
    }
    for(int i = 0; i < numOfSpots; i++){
        specular += calcSpotLight(sl[i],normalize(fs_in.nor),fs_in.pos, diffuse);
    }

	vec3 difColor = mix(texture(texDiffuseGrass, fs_in.tc * texScale).rgb, texture(texDiffuseDirt, fs_in.tc * texScale).rgb, 1.0 - (noise(fs_in.tc * texScale) * 0.5 + 0.5));
	FragColor = vec4(ambientColor + diffuse * difColor + vec3(0.0001) * specular, 1.0);
	EmissionColor = vec4(0.0);
}