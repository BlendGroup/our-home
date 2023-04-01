#ifndef __SHADER_LOADER__
#define __SHADER_LOADER__

#include<iostream>
#include<vector>
#include<unordered_map>
#include<initializer_list>
#include<GL/glew.h>
#include<GL/gl.h>

#define DL_SHADER_ES 100
#define DL_SHADER_CORE 101

class glshaderprogram {
private:
	static std::unordered_map<std::string, GLuint> shaderMap;
	GLuint programObject;
	std::unordered_map<std::string, GLint> uniforms;
public:
	glshaderprogram(std::initializer_list<std::string> shaderList, int version = 460, int profile = DL_SHADER_CORE);
	void use(void);
	GLint getUniformLocation(std::string uniformName);
	GLuint getProgramObject();
	void printUniforms(void);
	~glshaderprogram(void);
};

#endif