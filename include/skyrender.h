#pragma once

#include <math.h>
#ifndef __SKY_RENDER_SYSTEM_H__
#define __SKY_RENDER_SYSTEM_H__

#include<glmodelloader.h>
#include<string>
#include<vmath.h>
#include<debugcamera.h>
#include<CubeMapRenderTarget.h>
#include<glshaderloader.h>
#include<errorlog.h>

class SkyRenderSystem
{
public:
    SkyRenderSystem();
    ~SkyRenderSystem();

    void Render(vmath::mat4 pMat, camera *cam,float dt);
    
    void SetSunValue(float _sun) { sun = _sun; }
    void SetAnimateSun(bool b){animateSun = b;}
    void setSunAngle(float angle){
        sunAngle = angle;
        sunDir[1] = sinf(sunAngle);
        sunDir[2] = -cosf(sunAngle);
    }

private:
    glmodel* sphereMesh;
    glshaderprogram* skyShader;

    float innerRadius;
    float outerRadius;

    float g;
    float Kr;
    float Km;
    float sun;
    vmath::vec3 waveLength4;


    bool animateSun = false;
    float sunAngle;
    vmath::vec3 sunDir;
};

#endif // __SKY_RENDER_SYSTEM_H__
