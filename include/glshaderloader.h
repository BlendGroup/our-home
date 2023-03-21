#ifndef __SHADER_LOADER__
#define __SHADER_LOADER__

#include<iostream>
#include<vector>
#include<GL/glew.h>
#include<GL/gl.h>

#define DL_SHADER_ES 100
#define DL_SHADER_CORE 101

struct glprogram_dl {
	GLuint programObject;
};

struct glshader_dl {
	GLuint shaderObject;
	GLuint shaderProfile;
	GLuint shaderVersion;
};

std::string glshaderCreate(glshader_dl *shader, GLenum shaderType, std::string shaderFileName, GLuint shaderProfile = DL_SHADER_CORE, GLuint shaderVersion = 460);
void glshaderDestroy(glshader_dl *shader);

std::string glprogramCreate(glprogram_dl *program, std::string programName, std::vector<glshader_dl> shaderList);
void glprogramDestory(glprogram_dl *program);

#endif