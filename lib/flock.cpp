#include<global.h>
#include<flock.h>
#include<iostream>

#define PRESET_MAX_FORCE 3.0f
#define PRESET_MAX_SPEED 0.9f
#define PRESET_COHESION_RADIUS 1.1f
#define PRESET_ALIGNMENT_RADIUS 0.75f
#define PRESET_SEPARATION_RADIUS 10.0f
#define PRESET_ATTRACTION_WEIGHT 0.45f

using namespace std;
using namespace vmath;

/***************************** OpenCL Flocking *****************************/
Flock::Flock(size_t count, const vec3 &initAttractorPosition) {
	this->count = count,
	this->attractorPosition = initAttractorPosition;
	this->flockUpdateProgram = new glshaderprogram({"shaders/flock/flockupdate.comp"});
	this->flockProgram = new glshaderprogram({"shaders/flock/flock.vert", "shaders/color.frag"});
	this->colorProgram = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
	this->boidSphere = new sphere(5, 5, 1.0f);
	this->frameIndex = 0;

	glGenBuffers(2, this->flockBuffer);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->flockBuffer[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->count * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->flockBuffer[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, this->count * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);
	flock_member * ptr = (flock_member *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, this->count * sizeof(flock_member), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < this->count; i++) {
		ptr[i].position = initAttractorPosition + (vmath::vec3::random() - vmath::vec3(0.5f)) * 4.0f;
		ptr[i].velocity = vec3(0.0f, 0.0f, 0.0f);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

Flock::~Flock() {
	if(boidSphere) {
		delete boidSphere;
		boidSphere = NULL;
	}
	if(colorProgram) {
		delete colorProgram;
		colorProgram = NULL;
	}
	if(flockProgram) {
		delete flockProgram;
		flockProgram = NULL;
	}
}

void Flock::update(void) {
	this->flockUpdateProgram->use();
	glUniform3fv(this->flockUpdateProgram->getUniformLocation("goal"), 1, this->attractorPosition);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, this->flockBuffer[this->frameIndex]);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, this->flockBuffer[this->frameIndex ^ 1]);

	glDispatchCompute(64, 1, 1);
	this->frameIndex ^= 1;
}

void Flock::renderAsSpheres(const mat4& mMat, const vec4 &color, const vec4 &emissive, float scale) {
	this->flockProgram->use();
	glBindBufferBase(GL_UNIFORM_BUFFER, this->flockProgram->getUniformLocation("FlockBlock"), this->flockBuffer[this->frameIndex]);
	glUniformMatrix4fv(flockProgram->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(flockProgram->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(flockProgram->getUniformLocation("mMat"), 1, GL_FALSE, mMat);
	glUniform1f(flockProgram->getUniformLocation("scale"), scale);
	glUniform4fv(flockProgram->getUniformLocation("color"), 1, color);
	glUniform4fv(flockProgram->getUniformLocation("emissive"), 1, emissive);
	glUniform4fv(flockProgram->getUniformLocation("occlusion"), 1, vec4(0.0f, 0.0f, 0.0f, 0.0f));
	boidSphere->render(this->count);
}

void Flock::renderAttractorAsQuad(const mat4& mMat, const vec4 &color, const vec4 &emissive, float scale) {
	mat4 mvMatrix = programglobal::currentCamera->matrix() * mMat * translate(attractorPosition[0], attractorPosition[1], attractorPosition[2]);
	mvMatrix[0][0] = 1.0f;
	mvMatrix[0][1] = 0.0f;
	mvMatrix[0][2] = 0.0f;
	mvMatrix[1][0] = 0.0f;
	mvMatrix[1][1] = 1.0f;
	mvMatrix[1][2] = 0.0f;
	mvMatrix[2][0] = 0.0f;
	mvMatrix[2][1] = 0.0f;
	mvMatrix[2][2] = 1.0f;
	colorProgram->use();
	glUniformMatrix4fv(colorProgram->getUniformLocation("mvpMatrix"), 1, GL_FALSE, programglobal::perspective * mvMatrix * vmath::scale(scale));
	glUniform4fv(colorProgram->getUniformLocation("color"), 1, color);
	glUniform4fv(colorProgram->getUniformLocation("emissive"), 1, emissive);
	glUniform4fv(colorProgram->getUniformLocation("occlusion"), 1, vec4(0.0f, 0.0f, 0.0f, 0.0f));
	programglobal::shapeRenderer->renderQuad();
}
/*********************************** EOF ***********************************/
