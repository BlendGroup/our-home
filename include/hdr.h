#ifndef __HDR__
#define __HDR__

struct HDR
{
    unsigned int hdrFBO;
    unsigned int hdrRBO;
    unsigned int hdrTex;
    float exposure;
    float fade;
};

void setupProgramHDREffect();
void initHDREffect();
void renderHDREffect(HDR&);
void uninitHDREffect();
#endif