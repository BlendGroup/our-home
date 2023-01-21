#include<fstream>
#include<sstream>
#include"../include/glshaderloader.h"

using namespace std;

string glshaderCreate(glshader_dl *shader, GLenum shaderType, string shaderFileName, GLuint shaderProfile, GLuint shaderVersion) {
	string versionString = "#version " + to_string(shaderVersion);
	if(shaderProfile == DL_SHADER_CORE) {
		versionString += " core";
	} else if(shaderProfile == DL_SHADER_ES) {
		versionString += " es";
	}

	ifstream file(shaderFileName, ifstream::in);
	if(!file.is_open()) {
		return shaderFileName + ": file not found\n";
	}
	shader->shaderObject = glCreateShader(shaderType);
	string filestr(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
	int findVersion = filestr.find("#version");
	int findVersionCount = filestr.find('\n', findVersion) - findVersion;
	filestr.replace(findVersion, findVersionCount, versionString);
	const char* source = filestr.c_str();
	int flen = filestr.length();
	glShaderSource(shader->shaderObject, 1, &source, &flen);
	glCompileShader(shader->shaderObject);
	GLint compiledStatus;
	glGetShaderiv(shader->shaderObject, GL_COMPILE_STATUS, &compiledStatus);
	if(!compiledStatus) {
		char buffer[1024];
		glGetShaderInfoLog(shader->shaderObject, 1024, NULL, buffer);
		string s(buffer);
		return shaderFileName + ": compilation failed.\n" + s + "\n";
	}
	return "";
}

void glshaderDestroy(glshader_dl *shader) {
	glDeleteShader(shader->shaderObject);
}

string glprogramCreate(glprogram_dl *program, string programName, vector<glshader_dl> shaderList) {
	program->programObject = glCreateProgram();
	
	if(shaderList.empty()) {
		return programName + ": No shaders passed\n";
	}
	for(int i = 0; i < shaderList.size(); i++) {
		glAttachShader(program->programObject, shaderList[i].shaderObject);
	}
	glLinkProgram(program->programObject);
	GLint linkedStatus;
	glGetProgramiv(program->programObject, GL_LINK_STATUS, &linkedStatus);
	if(!linkedStatus) {
		char buffer[1024];
		glGetProgramInfoLog(program->programObject, 1024, NULL, buffer);
		string s(buffer);
		return programName + ": linking failed.\n" + s + "\n";
	}
	for(int i = 0; i < shaderList.size(); i++) {
		glDetachShader(program->programObject, shaderList[i].shaderObject);
	}
	return "";
}

void glprogramDestory(glprogram_dl *program) {
	glDeleteProgram(program->programObject);
}