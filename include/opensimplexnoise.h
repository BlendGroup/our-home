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
public:
	opensimplexnoise();
	void createNoiseTextureOnUniformInput(clglmem &outputNoise, noisetype type, const int* dim, const int* offset, float timeInterval, float amplitude, long seed);
};
