#pragma once
#include<vmath.h>
#include<clhelper.h>
#include<iostream>
#include<GL/glew.h>
#include<GL/gl.h>

class opensimplexnoise {
public:
	static GLuint createNoiseTexture2D(vmath::ivec2 dim, vmath::ivec2 offset, float timeInterval, float amplitude, long seed);
	static GLuint createFBMTexture2D(vmath::ivec2 dim, vmath::ivec2 offset, float timeInterval, int octave, long seed);
	static GLuint createTurbulenceFBMTexture2D(vmath::ivec2 dim, vmath::ivec2 offset, float timeInterval, int octave, float noiseoffset, long seed);
};
