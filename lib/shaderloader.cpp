#include<fstream>
#include<sstream>
#include"../include/glshaderloader.h"

using namespace std;

unordered_map<string, GLuint> glshaderprogram::shaderMap = {};

static unordered_map<string, GLenum> extensionMap = {
	{"vert", GL_VERTEX_SHADER},
	{"tesc", GL_TESS_CONTROL_SHADER},
	{"tese", GL_TESS_EVALUATION_SHADER},
	{"geom", GL_GEOMETRY_SHADER},
	{"frag", GL_FRAGMENT_SHADER},
	{"comp", GL_COMPUTE_SHADER},
};

glshaderprogram::glshaderprogram(initializer_list<std::string> shaderList, int version, int profile) {
	this->programObject = glCreateProgram();
	string versionString = "#version " + to_string(version);
	if(profile == DL_SHADER_CORE) {
		versionString += " core";
	} else if(profile == DL_SHADER_ES) {
		versionString += " es";
	}

	for(std::string shaderPath : shaderList) {
		ifstream file(shaderPath, ifstream::in);
		if(!file.is_open()) {
			throw "error: [" + string(__FILE__) + " " + to_string(__LINE__) + "] : '" + shaderPath + "' file not found.";
		}
		string filestr(istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));
		
		int findVersion = filestr.find("#version");
		int findVersionCount = filestr.find('\n', findVersion) - findVersion;
		filestr.replace(findVersion, findVersionCount, versionString);
		
		const char* source = filestr.c_str();
		int flen = filestr.length();
		GLuint shaderObject = glCreateShader(extensionMap[shaderPath.substr(shaderPath.find_last_of('.')+1)]);
		glShaderSource(shaderObject, 1, &source, &flen);
		glCompileShader(shaderObject);
		
		GLint compiledStatus;
		glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &compiledStatus);
		if(!compiledStatus) {
			char buffer[1024];
			glGetShaderInfoLog(shaderObject, 1024, NULL, buffer);
			string s(buffer);
		}
		glshaderprogram::shaderMap[shaderPath] = shaderObject;
		glAttachShader(this->programObject, shaderObject);
	}
	glLinkProgram(this->programObject);
	GLint linkedStatus;
	glGetProgramiv(this->programObject, GL_LINK_STATUS, &linkedStatus);
	if(!linkedStatus) {
		char buffer[1024];
		glGetProgramInfoLog(this->programObject, 1024, NULL, buffer);
		string s(buffer);
	}
	for(string shaderPath : shaderList) {
		glDetachShader(this->programObject, glshaderprogram::shaderMap[shaderPath]);
	}
}

glshaderprogram::~glshaderprogram() {

}