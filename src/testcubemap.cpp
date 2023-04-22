#include<testcubemap.h>
#include<GL/glew.h>
#include<GL/gl.h>
#include<vmath.h>
#include<glshaderloader.h>
#include<windowing.h>
#include<global.h>
#include<vector>
#include<utility>
#include<debugcamera.h>

using namespace vmath;
using namespace std;

static glshaderprogram* program;
static GLuint vao;
static GLuint vbo;
static vector<pair<vec3, vec3>> randomData;

static int cameraIndex = 0;

static mat4 lookatarray[6];
static GLuint fbo;
static GLuint texColor;
static GLuint texDepth;

void initTestRenderToCubemap() {
	const float cubeVerts[] = {
		1.0f,  1.0f,  1.0f,
       -1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,

		1.0f, -1.0f,  1.0f,
	   -1.0f,  1.0f,  1.0f,
	   -1.0f, -1.0f,  1.0f,

		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,

	   -1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
	   -1.0f, -1.0f, -1.0f,

	   -1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

	   -1.0f,  1.0f, -1.0f,
	   -1.0f, -1.0f, -1.0f,
	   -1.0f,  1.0f,  1.0f,

	   -1.0f,  1.0f,  1.0f,
	   -1.0f, -1.0f, -1.0f,
	   -1.0f, -1.0f,  1.0f,

		1.0f,  1.0f, -1.0f,
	   -1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
	   -1.0f,  1.0f, -1.0f,
	   -1.0f,  1.0f,  1.0f,

	   -1.0f, -1.0f,  1.0f,
	   -1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		
		1.0f, -1.0f,  1.0f,
	   -1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f
	};
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glCreateBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenTextures(1, &texColor);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texColor);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &texDepth);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texDepth);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texColor, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texDepth, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	srand(0);
	for(int i = 0; i < 300; i++) {
		randomData.push_back({
			vec3(
				(float)rand() / RAND_MAX * 60.0f - 30.0f, 
				(float)rand() / RAND_MAX * 60.0f - 30.0f, 
				(float)rand() / RAND_MAX * 60.0f - 30.0f
			), 
			vec3(
				(float)rand() / RAND_MAX, 
				(float)rand() / RAND_MAX, 
				(float)rand() / RAND_MAX
			)
		});
	}

	static mat4 perspectiveForCubemap = perspective(90.0f, 1.0f, 0.1f, 100.0f);
	debugCamera* cam;
	cam = new debugCamera(vec3(0.0f, 0.0f, 0.0f), -90.0f, 0.0f), //Front
	lookatarray[0] = perspectiveForCubemap * cam->matrix();
	delete cam;
	cam = new debugCamera(vec3(0.0f, 0.0f, 0.0f), 90.0f, 0.0f), //Back
	lookatarray[1] = perspectiveForCubemap * cam->matrix();
	delete cam;
	cam = new debugCamera(vec3(0.0f, 0.0f, 0.0f), 90.0f, -90.0f), //Bottom 
	lookatarray[2] = perspectiveForCubemap * cam->matrix();
	delete cam;
	cam = new debugCamera(vec3(0.0f, 0.0f, 0.0f), 90.0f, 90.0f), //Top
	lookatarray[3] = perspectiveForCubemap * cam->matrix();
	delete cam;
	cam = new debugCamera(vec3(0.0f, 0.0f, 0.0f), 180.0f, 0.0f), //Left
	lookatarray[4] = perspectiveForCubemap * cam->matrix();
	delete cam;
	cam = new debugCamera(vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f), //Right
	lookatarray[5] = perspectiveForCubemap * cam->matrix();
	delete cam;
}

void setupProgramTestRenderToCubemap() {
	program = new glshaderprogram({"shaders/debug/color.vert", "shaders/debug/color.geom", "shaders/debug/color.frag"});
}

void renderTestRenderToCubemap(camera* cam) {

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	program->use();
	// if(cameraIndex == 0) {
	// 	glUniformMatrix4fv(program->getUniformLocation("mvpMat"), 1, GL_FALSE, programglobal::perspective * cam->matrix());
	// } else {
		glUniformMatrix4fv(program->getUniformLocation("mvpMat[0]"), 6, GL_FALSE, (float*)&lookatarray[0][0][0]);
	// }
	for(int i = 0; i < randomData.size(); i++) {
		glUniform3fv(program->getUniformLocation("trans"), 1, randomData[i].first);
		glUniform3fv(program->getUniformLocation("color"), 1, randomData[i].second);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void keyboardFuncTestRenderToCubemap(int key) {
	switch(key) {
	case XK_0: case XK_1: case XK_2: case XK_3: case XK_4: case XK_5: case XK_6:
		cameraIndex = key - XK_0;
		break;
	}
}

void uninitTestRenderToCubemap() {

}