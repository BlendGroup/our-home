#include<godrays.h>
#include<testgodrays.h>

using namespace std;
using namespace vmath;

#define GODRAYS_RENDERPASS_WIDTH  960
#define GODRAYS_RENDERPASS_HEIGHT 540

static godrays* godRays;
static glshaderprogram* colorProgram = NULL;
static GLuint vaoCube, vboCube;
static GLuint vaoPlane, vboPlane;
static const vmath::mat4 godraysPerspective = vmath::perspective(
	45.0f,
	(float)GODRAYS_RENDERPASS_WIDTH / (float)GODRAYS_RENDERPASS_HEIGHT,
	0.1f,
	1000.0f
);
static const vec4 posPlane = vec4(0.0f, 0.0f, -5.0f, 1.0f);
static const mat4 mMatCube = scale(0.25f, 0.25f, 0.25f);
static const mat4 mMatPlane = translate(posPlane[0], posPlane[1], posPlane[2]);

void setupProgramTestGodrays(void) {
	try {
		colorProgram = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void initTestGodrays(void) {
	try {
		godRays = new godrays(GODRAYS_RENDERPASS_WIDTH, GODRAYS_RENDERPASS_HEIGHT);
		
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

static void renderTestGodraysScene(camera *cam, bool isOcclusionPass) {
	try {
		if(isOcclusionPass) {
			// all occluding objects
			godRays->occlusionPass(godraysPerspective * cam->matrix() * mMatCube, false);
			glBindVertexArray(vaoCube);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			// all emissive objects
			godRays->occlusionPass(godraysPerspective * cam->matrix() * mMatPlane, true);
			glBindVertexArray(vaoPlane);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		else {
			colorProgram->use();
			
			glUniformMatrix4fv(colorProgram->getUniformLocation("mvpMatrix"), 1, GL_FALSE, programglobal::perspective * cam->matrix() * mMatCube);
			glUniform4fv(colorProgram->getUniformLocation("color"), 1, vec4(0.1f, 0.1f, 0.1f, 1.0f));
			glBindVertexArray(vaoCube);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			glUniformMatrix4fv(colorProgram->getUniformLocation("mvpMatrix"), 1, GL_FALSE, programglobal::perspective * cam->matrix() * mMatPlane);
			glUniform4fv(colorProgram->getUniformLocation("color"), 1, vec4(1.0f, 1.0f, 1.0f, 1.0f));
			glBindVertexArray(vaoPlane);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void renderTestGodrays(camera *cam, int winWidth, int winHeight) {
	try {
		GLint renderingFbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &renderingFbo);

		// occlusion pass
	    glBindFramebuffer(GL_FRAMEBUFFER, godRays->getFbo());
		glViewport(0, 0, GODRAYS_RENDERPASS_WIDTH, GODRAYS_RENDERPASS_WIDTH);
		renderTestGodraysScene(cam, true);

		// regular pass
	    glBindFramebuffer(GL_FRAMEBUFFER, renderingFbo);
		glViewport(0, 0, winWidth, winHeight);
		renderTestGodraysScene(cam, false);

		// additive blending pass
		godRays->render(
			programglobal::perspective * cam->matrix() * mMatPlane,
			posPlane,
			1.0f,
			0.01f,
			1.0f,
			1.0f,
			100
		);
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void uninitTestGodrays() {
	if(colorProgram) {
		delete colorProgram;
		colorProgram = NULL;
	}
	if(godRays) {
		delete godRays;
		godRays = NULL;
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
