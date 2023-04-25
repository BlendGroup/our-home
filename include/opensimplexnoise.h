#pragma once
#include<vmath.h>
#include<clhelper.h>
#include<iostream>

enum noisetype {
	Noise2D,
	Noise3D,
	Noise4D
};

class opensimplexnoise {
private:
    clglcontext* context;
    cl_kernel noiseKernel;
    noisetype type;
public:
	opensimplexnoise(noisetype type);
	void createNoiseTextureOnUniformInput(clglmem &outputNoise, const int* dim, const int* offset, float timeInterval, float amplitude, long seed);
};

