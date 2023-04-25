#pragma once
#include<vmath.h>
#include<clhelper.h>
#include<iostream>
#include<GL/glew.h>
#include<GL/gl.h>

enum noisetype {
	Noise2D,
	Noise3D,
	Noise4D
};

class opensimplexnoise {
public:
	opensimplexnoise();
	GLuint createNoiseTextureOnUniformInput(noisetype type, const int* dim, const int* offset, float timeInterval, float amplitude, long seed);
};
