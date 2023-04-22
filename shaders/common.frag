#version 460 core

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

layout(location = 0)out vec4 fragColor;

struct material_t{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
    float opacity;
};

struct light_t{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 position;
    vec3 attenuation;
    vec2 cutoff;
    vec3 direction;
}

in VS_OUT {
    vec3 P;
	vec3 N;
	vec2 Tex;
} fs_in;

uniform vec3 viewPos;
uniform bool isTextured;
uniform bool isBlend;

uniform material_t material;
uniform light_t light[15];
uniform int numOfLights;

void main(void) {

    vec3 color = vec3(0.0);
    float alpha;
    if(isTextured){
        vec4 t = texture(diffuseTexture,fs_in.Tex);
        color = t.rgb * material.diffuse;
        alpha = t.a * material.opacity;
    }
    else{
        color = material.diffuse;
        alpha = material.opacity;
    }

    if(!isBlend){
        alpha = 1.0;
    }

    if(numOfLights > 0)
    {
        vec3 fcolor = vec3(0.0);
        vec3 N = normalize(fs_in.N);
        vec3 V = normalize(viewPos - fs_in.P);

        for(int i = 0; i < numOfLights; i++)
        {
            vec3 L = normalize(light[i].position - fs_in.P);
            vec3 R = reflect(-L,N);
            float attenuation = 1.0;
            float intensity = 1.0;
            if(light[i].attenuation.x != 0.0){
                float dist = length(light[i].position - P);
                attenuation = 1.0 / (light[i].attenuation.x + light[i].attenuation.y * dist + light[i].attenuation.z * (dist*dist));
            }
            if(light[i].cutoff.x != 0.0){
                float theta = dot(L,normalize(-light[i].direction));
                float epsilon = (cos(radians(light[i].cutoff.x)) - cos(radians(light[i].cutoff.y)));
                intensity = clamp((theta - cos(radians(lights[i].cutoff.y))) / epsilon,0.0,1.0);
            }
            vec3 ambient = attenuation * color * light[i].ambient * material.ambient;
            vec3 diffuse = intensity * attenuation * max(dot(N,L),0.0) * color * light[i].diffuse;
            vec3 specular = intensity * attenuation * pow(max(dot(R,V),0.0),material.shininess) * light[i].specular * material.specular;
            fcolor += ambient + diffuse + specular;
        }
        color = fcolor;
    }
    fragColor = vec4(color,alpha);
}