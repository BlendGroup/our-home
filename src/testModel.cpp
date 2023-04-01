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
static glshaderprogram* program1;
static rvModel* backpack;
static rvModel* vampire;
static bool gStatic;

struct
{
    GLfloat cameraYaw;
    GLfloat cameraPitch;
    vec3 cameraFront;
    vec3 cameraPosition;
    vec3 cameraUp;
}DebugCam;

void setupProgramTestModel() {
	try
	{
		if(gStatic)
		{
			program = new glshaderprogram({"src/shaders/modelStatic.vert", "src/shaders/modelAnimation.frag"});
			//program->printUniforms();
		}
		else 
		{
			program1 = new glshaderprogram({"src/shaders/modelAnimation.vert", "src/shaders/modelAnimation.frag"});
			vampire->initShaders(program1);
			//program1->printUniforms();
		}
	}
	catch(std::string errorString)
	{
		cout<<errorString<<endl;
	}

}

void initTestModel(string path, bool staticModel) {
	gStatic = staticModel;

	if(staticModel)
	{
		backpack = new rvModel();
		backpack->loadModel(path);
	}
	else {
		vampire = new rvModel();
		vampire->loadModel(path);	
	}
    DebugCam.cameraYaw = -90.0f;
    DebugCam.cameraPitch = 0.0f;
    DebugCam.cameraFront = vec3(0.0f, 0.0f, -1.0f);
    DebugCam.cameraPosition = vec3(0.0f,0.0f,5.0f);
    DebugCam.cameraUp = vec3(0.0f,1.0f,0.0f);
}

void renderTestModel(mat4 perspective) {

	static float t = 0.0f;
	t += 0.0065f;
	mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();
    viewMatrix = vmath::lookat(DebugCam.cameraPosition,DebugCam.cameraFront,DebugCam.cameraUp);

	try
	{
		// waiting for debug cam
		modelMatrix *= vmath::translate(0.0f, 0.0f, -5.0f);
		if(gStatic)
		{
			program->use();
			glUniformMatrix4fv(program->getUniformLocation("u_Model"), 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(program->getUniformLocation("u_View"), 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(program->getUniformLocation("u_Projection"), 1, GL_FALSE, perspective);
			backpack->draw(program, t);
		}
		else 
		{
			program1->use();
			glUniformMatrix4fv(glGetUniformLocation(program1->getProgramObject(),"u_Model"), 1, GL_FALSE, modelMatrix);
			glUniformMatrix4fv(glGetUniformLocation(program1->getProgramObject(),"u_View"), 1, GL_FALSE, viewMatrix);
			glUniformMatrix4fv(glGetUniformLocation(program1->getProgramObject(),"u_Projection"), 1, GL_FALSE, perspective);
			vampire->draw(program1, t);		
		}
	}
	catch(string errorString)
	{
		cout<<errorString<<endl;
	}
}

void keyboardfuncModel(int key) {
	vec3 dir;
	switch(key) {

		case XK_Up:
			dir = vec3(0.0f);
			dir = DebugCam.cameraFront * vec3(0.3f,0.3f,0.3f);
			DebugCam.cameraPosition += dir;
		break;
		case XK_Down:
			dir = vec3(0.0f);
			dir = DebugCam.cameraFront * vec3(0.3f,0.3f,0.3f);
			DebugCam.cameraPosition -= dir;
		break;
		case XK_Right:
			dir = cross(DebugCam.cameraFront, DebugCam.cameraUp);
			dir = normalize(dir);
			dir *= vec3(0.3f,0.3f,0.3f);
			DebugCam.cameraPosition += dir;
		break;
		case XK_Left:
			dir = cross(DebugCam.cameraFront, DebugCam.cameraUp);
			dir = normalize(dir);
			dir *= vec3(0.3f,0.3f,0.3f);
			DebugCam.cameraPosition -= dir;
		break;
	}
}


void uninitTestModel() {
	
	if(backpack)
	{
		backpack->ModelCleanUp();
		backpack = nullptr;
	}
	if(vampire)
	{
		vampire->ModelCleanUp();
		vampire = nullptr;
	}
	delete program;
	delete program1;
}
