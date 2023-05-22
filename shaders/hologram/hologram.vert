#version 460 core

layout(location = 0)in vec4 vPos;
layout(location = 1)in vec3 vNor;
layout(location = 2)in vec2 vTex;

uniform mat4 pMat;
uniform mat4 vMat;
uniform mat4 mMat;

uniform float gTime;
uniform float GlitechIntensity; 
uniform float GlitchSpeed;

out VS_OUT{
    vec3 P;
    vec3 view;
    vec3 N;
    vec2 Tex;
}vs_out;

void main(void){

    vec4 eye = vMat * mMat * vPos;
    mat3 normalMatrix = mat3(vMat * mMat);
    vs_out.N = normalize(normalMatrix * vNor);
    vs_out.view = normalize(-eye.xyz);

    vs_out.P = vec3(mMat * vPos);
    vs_out.P.x += GlitechIntensity * step(0.5,sin(gTime*2.0+vPos.y*1.0)) * step(0.99,sin(gTime * GlitchSpeed * 0.5));
    vs_out.Tex = vTex;
    gl_Position = pMat * vMat * mMat * vPos;
}
