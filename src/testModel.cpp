#include <bits/types/FILE.h>
#include <cstddef>
#include <X11/keysym.h>
#include<iostream>
#include <ostream>
#include <string>

#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/vmath.h"
#include"../include/rvModel.hpp"
#include"../include/testModel.h"

using namespace std;
using namespace vmath;

static glshaderprogram* program;
//static glshaderprogram* program1;
static rvModel* backpack;
//static rvModel* vampire;
GLuint tempVao;

void setupProgramTestModel() {
	try
	{
		program = new glshaderprogram({"src/shaders/modelStatic.vert", "src/shaders/modelAnimation.frag"});
		//program1 = new glshaderprogram({"src/shaders/modelAnimation.vert", "src/shaders/modelAnimation.frag"});
		//vampire->initShaders(program1);
		program->printUniforms();
		//program1->printUniforms();
	}
	catch(std::string errorString)
	{
		cout<<errorString<<endl;
	}

}

void initTestModel(string path) {

	backpack = new rvModel();
	backpack->loadModel(path);
	//vampire = new rvModel();
	//vampire->loadModel("resources/vampire/Idle.dae");
}

void renderTestModel(mat4 perspective) {
	static float t = 0.0f;
	t += 1.0f;
	mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    viewMatrix = vmath::lookat(vec3(0.0f,0.0f,5.0f),vec3(0.0f,0.0f,-1.0f) ,vec3(0.0f,1.0f,0.0f));

	try
	{
		// waiting for debug cam
		modelMatrix *= vmath::translate(0.0f, 0.0f, -5.0f) * vmath::rotate(vmath::radians(t*5.0f),vec3(0.0f,1.0f,0.0f));
		program->use();
		glUniformMatrix4fv(program->getUniformLocation("u_Model"), 1, GL_FALSE, modelMatrix);
		glUniformMatrix4fv(program->getUniformLocation("u_View"), 1, GL_FALSE, viewMatrix);
		glUniformMatrix4fv(program->getUniformLocation("u_Projection"), 1, GL_FALSE, perspective);
		backpack->draw(program, t);

		/*
		modelMatrix = mat4::identity();
		modelMatrix *= vmath::translate(0.0f, 0.0f, -5.0f);
		program1->use();
		glUniformMatrix4fv(program1->getUniformLocation("u_Model"), 1, GL_FALSE, modelMatrix);
		glUniformMatrix4fv(program1->getUniformLocation("u_View"), 1, GL_FALSE, viewMatrix);
		glUniformMatrix4fv(program1->getUniformLocation("u_Projection"), 1, GL_FALSE, perspective);
		vampire->draw(program1, t);
		*/
	}
	catch(string errorString)
	{
		cout<<errorString<<endl;
	}
}

void uninitTestModel() {
	if(backpack)
	{
		backpack->ModelCleanUp();
		backpack = nullptr;
	}
/*
	if(vampire)
	{
		vampire->ModelCleanUp();
		vampire = nullptr;
	}
*/
	delete program;
//	delete program1;
}
