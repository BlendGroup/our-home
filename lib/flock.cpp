#include<flock.h>
#include<iostream>

using namespace std;
using namespace vmath;

/***************************** OpenCL Flocking *****************************/
Flock::Flock(size_t count, const vec4 &initAttractorPosition)
: count(count),
  attractorPosition(initAttractorPosition),
  cohesionRadius(PRESET_COHESION_RADIUS),
  alignmentRadius(PRESET_ALIGNMENT_RADIUS),
  separationRadius(PRESET_SEPARATION_RADIUS),
  maxDistanceFromAttractor(PRESET_MAX_DISTANCE_FROM_ATTRACTOR),
  maxSpeed(PRESET_MAX_SPEED),
  maxForce(PRESET_MAX_FORCE)
  {
    this->flockBufferCLGL = programglobal::oclContext->createCLGLBuffer(count * sizeof(boid_t), GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT, CL_MEM_READ_WRITE);
    glBindBuffer(GL_ARRAY_BUFFER, this->flockBufferCLGL.gl);
    boid_t *pFlockBuffer = (boid_t *)glMapBufferRange(GL_ARRAY_BUFFER, 0, count * sizeof(boid_t), GL_MAP_WRITE_BIT);
	auto randomVec4Offset = [] (float minLength, float maxLength) -> vec4 {
		float x = fmodf(rand(), 100.0f);
		x = (rand() & 1) ? x : -x;
		float y = fmodf(rand(), 100.0f);
		y = (rand() & 1) ? y : -y;
		float z = fmodf(rand(), 100.0f);
		z = (rand() & 1) ? z : -z;	
		float t = (float)rand() / (float)RAND_MAX;
		vec3 out = (minLength + (maxLength - minLength) * t) * normalize(vec3(x, y, z));
		return vec4(out, 0.0f);
	};
	srand(time(NULL));
    for(int i = 0; i < count; i++) {
		srand(rand());
        pFlockBuffer[i].position = initAttractorPosition + randomVec4Offset(0.0f, 5.0f);
		srand(rand());
        pFlockBuffer[i].velocity = randomVec4Offset(0.01f, 0.02f);
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
    programglobal::oclContext->setKernelParameters(
        flockUpdateKernel,
        { clkernelparam(0, this->flockBufferCLGL.cl),
        clkernelparam(1, this->count),
        clkernelparam(2, this->cohesionRadius),
        clkernelparam(3, this->alignmentRadius),
        clkernelparam(4, this->separationRadius),
        clkernelparam(5, this->attractorPosition),
        clkernelparam(6, this->maxDistanceFromAttractor),
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
    boidSphere->renderInstanced(this->count);
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
