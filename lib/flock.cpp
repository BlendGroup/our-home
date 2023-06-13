#include "global.h"
#include<flock.h>
#include<iostream>
#include<random>

#define PRESET_MAX_FORCE 3.0f
#define PRESET_MAX_SPEED 0.9f
#define PRESET_COHESION_RADIUS 1.1f
#define PRESET_ALIGNMENT_RADIUS 0.75f
#define PRESET_SEPARATION_RADIUS 10.0f
#define PRESET_ATTRACTION_WEIGHT 0.45f

using namespace std;
using namespace vmath;

/***************************** OpenCL Flocking *****************************/
Flock::Flock(size_t count, const vec3 &initAttractorPosition)
: count(count),
  attractorPosition(initAttractorPosition),
  cohesionRadius(PRESET_COHESION_RADIUS),
  alignmentRadius(PRESET_ALIGNMENT_RADIUS),
  separationRadius(PRESET_SEPARATION_RADIUS),
  attractionWeight(PRESET_ATTRACTION_WEIGHT),
  maxSpeed(PRESET_MAX_SPEED),
  maxForce(PRESET_MAX_FORCE)
  {
    this->flockBufferCLGL = programglobal::oclContext->createCLGLBuffer(count * sizeof(boid_t), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT, CL_MEM_READ_WRITE);
    glBindBuffer(GL_ARRAY_BUFFER, this->flockBufferCLGL.gl);
    boid_t *pFlockBuffer = (boid_t *)glMapBufferRange(GL_ARRAY_BUFFER, 0, count * sizeof(boid_t), GL_MAP_WRITE_BIT);
	
    for(int i = 0; i < count; i++) {
        pFlockBuffer[i].position = vec4(initAttractorPosition + programglobal::randgen->getRandomVector3(0.0f, 5.0f), 1.0);
        pFlockBuffer[i].velocity = vec4(programglobal::randgen->getRandomVector3(0.01f, 0.02f), 0.0f);
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    flockProgram = new glshaderprogram({"shaders/flock/flock.vert", "shaders/color.frag"});
    colorProgram = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
    boidSphere = new sphere(5, 5, 1.0f);
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
    if(this->flockBufferCLGL.gl) {
        glDeleteBuffers(1, &this->flockBufferCLGL.gl);
        this->flockBufferCLGL.gl = 0;
    }
    if(this->flockBufferCLGL.cl) {
        clReleaseMemObject(this->flockBufferCLGL.cl);
        this->flockBufferCLGL.cl = NULL;
    }
}

void Flock::update(void) {
    cl_kernel flockUpdateKernel = programglobal::oclContext->getKernel("flock_update");
    vec4 attractor = vec4(this->attractorPosition, 0.0f);
    programglobal::oclContext->setKernelParameters(
        flockUpdateKernel,
        { clkernelparam(0, this->flockBufferCLGL.cl),
        clkernelparam(1, this->count),
        clkernelparam(2, this->cohesionRadius),
        clkernelparam(3, this->alignmentRadius),
        clkernelparam(4, this->separationRadius),
        clkernelparam(5, attractor),
        clkernelparam(6, this->attractionWeight),
        clkernelparam(7, this->maxSpeed),
        clkernelparam(8, this->maxForce) }
    );
    size_t globalWorkSize = this->count;
    programglobal::oclContext->runCLKernel(flockUpdateKernel, 1, &globalWorkSize, NULL, {this->flockBufferCLGL});
    CLErr(clhelpererr = clFinish(programglobal::oclContext->getCommandQueue()));
}

void Flock::renderAsSpheres(const vec4 &color, const vec4 &emissive, float scale) {
    flockProgram->use();
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, this->flockBufferCLGL.gl);
    glUniformMatrix4fv(flockProgram->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
    glUniformMatrix4fv(flockProgram->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
    glUniform1f(flockProgram->getUniformLocation("scale"), scale);
    glUniform4fv(flockProgram->getUniformLocation("color"), 1, color);
    glUniform4fv(flockProgram->getUniformLocation("emissive"), 1, emissive);
    glUniform4fv(flockProgram->getUniformLocation("occlusion"), 1, vec4(0.0f, 0.0f, 0.0f, 0.0f));
    boidSphere->render(this->count);
}

void Flock::renderAttractorAsQuad(const vec4 &color, const vec4 &emissive, float scale) {
    mat4 mvMatrix = programglobal::currentCamera->matrix() * translate(attractorPosition[0], attractorPosition[1], attractorPosition[2]);
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
