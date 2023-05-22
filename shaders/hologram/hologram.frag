#version 460 core

//input
in VS_OUT {
    vec3 P;
    vec3 view;
    vec3 N;
    vec2 Tex;
} fs_in;

uniform vec4 MainColor;
uniform vec4 RimColor;
uniform float gTime;
uniform float BarSpeed;
uniform float BarDistance;
uniform float alpha;
uniform float FlickerSpeed;
uniform float RimPower;
uniform float GlowSpeed;
uniform float GlowDistance;

layout(location = 0)out vec4 FragColor;
layout(location = 1)out vec4 EmmisiveColor;


float rand(float n){
    return fract(sin(n) * 43758.5453123);
}

float noise(float p){
    float fl = floor(p);
    float fc = fract(p);
    return mix(rand(fl),rand(fl+1.0),fc);
}

void main(void){

    float val = gTime * BarSpeed + fs_in.P.y * BarDistance;
    float bars = step(val - floor(val),0.5) * 0.65;
    float flicker = clamp(noise(gTime * FlickerSpeed),0.65,1.0);
    float rim = 1.0 - clamp(dot(fs_in.view,fs_in.N),0.0,1.0);
    vec4 rimColor = RimColor * pow(rim,RimPower);
    float glow = fs_in.P.y * GlowDistance - gTime * GlowSpeed;

    FragColor = MainColor + rimColor + (0.35 * MainColor);
    FragColor.a = 0.1;
    //FragColor = vec4(1.0,0.0,0.0,1.0);
    if(FragColor.a <= 0.01)
        discard;
	EmmisiveColor = vec4(FragColor.rgb, 1.0) *  0.2;
}


