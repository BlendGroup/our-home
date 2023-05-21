#include<godrays.h>
#include<testgodrays.h>

using namespace std;
using namespace vmath;

static godrays* godraysPasses;
static glshaderprogram* colorProgram = NULL;
static GLuint vaoCube, vboCube;
static GLuint vaoPlane, vboPlane;

void setupProgramTestGodrays(int winWidth, int winHeight) {
	try {
		GLint defaultFbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFbo);
		godraysPasses = new godrays(defaultFbo, winWidth, winHeight);
		colorProgram = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void initTestGodrays() {
	try {
		const float planeVerts[] = {
			-1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,

			1.0f, 1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f
		};
		glCreateVertexArrays(1, &vaoPlane);
		glBindVertexArray(vaoPlane);
		glCreateBuffers(1, &vboPlane);
		glBindBuffer(GL_ARRAY_BUFFER, vboPlane);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

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
		glCreateVertexArrays(1, &vaoCube);
		glBindVertexArray(vaoCube);
		glCreateBuffers(1, &vboCube);
		glBindBuffer(GL_ARRAY_BUFFER, vboCube);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

	} catch(string errorString) {
		throwErr(errorString);
	}
}

void renderTestGodrays(camera *cam) {
	try {
		// occlusion pass
		godraysPasses->beginOcclusionPass(cam->matrix() * scale(0.25f, 0.25f, 0.25f), false);
		glBindVertexArray(vaoCube);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		godraysPasses->beginOcclusionPass(cam->matrix() * translate(0.0f, 0.0f, -5.0f), true);
		glBindVertexArray(vaoPlane);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		godraysPasses->endOcclusionPass();

		// regular pass
		colorProgram->use();

		glUniformMatrix4fv(colorProgram->getUniformLocation("mvpMatrix"), 1, GL_FALSE, programglobal::perspective * cam->matrix() * scale(0.25f, 0.25f, 0.25f));
		glUniform4fv(colorProgram->getUniformLocation("color"), 1, vec4(0.1f, 0.1f, 0.1f, 1.0f));
		glBindVertexArray(vaoCube);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		glUniformMatrix4fv(colorProgram->getUniformLocation("mvpMatrix"), 1, GL_FALSE, programglobal::perspective * cam->matrix() * translate(0.0f, 0.0f, -5.0f));
		glUniform4fv(colorProgram->getUniformLocation("color"), 1, vec4(1.0f, 1.0f, 1.0f, 1.0f));
		glBindVertexArray(vaoPlane);
		glDrawArrays(GL_TRIANGLES, 0, 6);

	} catch(string errorString) {
		throwErr(errorString);
	}
}

void uninitTestGodrays() {
	if(colorProgram) {
		delete colorProgram;
		colorProgram = NULL;
	}
	if(godraysPasses) {
		delete godraysPasses;
		godraysPasses = NULL;
	}
	if(vboPlane) {
		glDeleteBuffers(1, &vboPlane);
		vboPlane = 0;
	}
	if(vaoPlane) {
		glDeleteVertexArrays(1, &vaoPlane);
		vaoPlane = 0;
	}
	if(vboCube) {
		glDeleteBuffers(1, &vboCube);
		vboCube = 0;
	}
	if(vaoCube) {
		glDeleteVertexArrays(1, &vaoCube);
		vaoCube = 0;
	}
}
