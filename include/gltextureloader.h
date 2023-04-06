#ifndef __TEXTURE_LOADER__
#define __TEXTURE_LOADER__

#include<string>
#include<GL/glew.h>
#include<GL/gl.h>

GLuint createTexture2D(std::string filename, GLint minFilter = GL_NEAREST_MIPMAP_NEAREST, GLint magFilter = GL_NEAREST, GLint wrapS = GL_REPEAT, GLint wrapT = GL_REPEAT);

#endif