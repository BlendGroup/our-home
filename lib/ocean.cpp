#include<ocean.h>
#include<iostream>
#include<vmath.h>
#include<global.h>

using namespace std;
using namespace vmath;

#define GEOMETRY_ORIGIN vec2(-1.0f, -1.0f)
#define GEOMETRY_RESOLUTION 256
#define GEOMETRY_SIZE 2.0f
#define RESOLUTION 512

static GLuint vaoEmpty;

GLuint createFramebufferFromTexture(GLuint attachment) {
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, attachment, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return framebuffer;
}

GLuint createTextureFromParams(GLenum internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height, void* data, GLenum wrapS, GLenum wrapT, GLenum minFilter, GLenum magFilter) {
    GLuint texture;
	glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

ocean::ocean(vec2 wind, float choppiness, int size) {
	this->wind = wind;
	this->choppiness = choppiness;
	this->size = size;
	this->pingPhase = true;
	
	string fullscreenVertexShader = "shaders/fsquad.vert";

	this->horizontalSubtransformProgram = new glshaderprogram({ fullscreenVertexShader, "shaders/ocean/horizontalSubtransform.frag" });
	this->horizontalSubtransformProgram->use();
	glUniform1f(this->horizontalSubtransformProgram->getUniformLocation("u_transformSize"), RESOLUTION);

	this->verticalSubtransformProgram = new glshaderprogram({fullscreenVertexShader, "shaders/ocean/verticalSubtransform.frag"});
	this->verticalSubtransformProgram->use();
	glUniform1f(this->verticalSubtransformProgram->getUniformLocation("u_transformSize"), RESOLUTION);
	
	this->initialSpectrumProgram = new glshaderprogram({fullscreenVertexShader, "shaders/ocean/initialSpectrum.frag"});
	this->initialSpectrumProgram->use();
	glUniform1f(this->initialSpectrumProgram->getUniformLocation("u_resolution"), RESOLUTION);

	
	this->phaseProgram = new glshaderprogram({fullscreenVertexShader, "shaders/ocean/phase.frag"});
	this->phaseProgram->use();
	glUniform1f(this->phaseProgram->getUniformLocation("u_resolution"), RESOLUTION);

	this->spectrumProgram = new glshaderprogram({fullscreenVertexShader, "shaders/ocean/spectrum.frag"});
	this->spectrumProgram->use();
	glUniform1f(this->spectrumProgram->getUniformLocation("u_resolution"), RESOLUTION);

	this->normalMapProgram = new glshaderprogram({fullscreenVertexShader, "shaders/ocean/normalMap.frag"});
	this->normalMapProgram->use();
	glUniform1f(this->normalMapProgram->getUniformLocation("u_resolution"), RESOLUTION);

	glUseProgram(0);

	vector<float> quad = {
		-1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0
	};

	glGenVertexArrays(1, &vaoEmpty);	
	glGenVertexArrays(1, &this->vaoOcean);
	glBindVertexArray(this->vaoOcean);
	vector<float> oceanData;
	for(int zIndex = 0; zIndex < GEOMETRY_RESOLUTION; zIndex += 1) {
		for(int xIndex = 0; xIndex < GEOMETRY_RESOLUTION; xIndex += 1) {
			oceanData.push_back(((float)xIndex * GEOMETRY_SIZE) / (GEOMETRY_RESOLUTION - 1.0f) + GEOMETRY_ORIGIN[0]);
			oceanData.push_back(0.0f);
			oceanData.push_back(((float)zIndex * GEOMETRY_SIZE) / (GEOMETRY_RESOLUTION - 1.0f) + GEOMETRY_ORIGIN[1]);
			oceanData.push_back((float)xIndex / (GEOMETRY_RESOLUTION - 1));
			oceanData.push_back((float)zIndex / (GEOMETRY_RESOLUTION - 1));
		}
	}
	
	vector<unsigned short> oceanIndices;
	for(int zIndex = 0; zIndex < GEOMETRY_RESOLUTION - 1; zIndex += 1) {
		for(int xIndex = 0; xIndex < GEOMETRY_RESOLUTION - 1; xIndex += 1) {
			int topLeft = zIndex * GEOMETRY_RESOLUTION + xIndex,
				topRight = topLeft + 1,
				bottomLeft = topLeft + GEOMETRY_RESOLUTION,
				bottomRight = bottomLeft + 1;

			oceanIndices.push_back(topLeft);
			oceanIndices.push_back(bottomLeft);
			oceanIndices.push_back(bottomRight);
			oceanIndices.push_back(bottomRight);
			oceanIndices.push_back(topRight);
			oceanIndices.push_back(topLeft);
		}
	}

	glGenBuffers(1, &this->oceanBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, this->oceanBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * oceanData.size(), oceanData.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	this->countOfIndicesOcean = oceanIndices.size();
	glGenBuffers(1, &this->oceanIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oceanIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * oceanIndices.size(), oceanIndices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);

	this->initialSpectrumTexture = createTextureFromParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, RESOLUTION, RESOLUTION, NULL, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
	this->pongPhaseTexture = createTextureFromParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, RESOLUTION, RESOLUTION, NULL, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	this->spectrumTexture = createTextureFromParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, RESOLUTION, RESOLUTION, NULL, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	this->displacementMap = createTextureFromParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, RESOLUTION, RESOLUTION, NULL, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	this->normalMap = createTextureFromParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, RESOLUTION, RESOLUTION, NULL, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	this->pingTransformTexture = createTextureFromParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, RESOLUTION, RESOLUTION, NULL, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	this->pongTransformTexture = createTextureFromParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, RESOLUTION, RESOLUTION, NULL, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

	float phaseArray[RESOLUTION * RESOLUTION * 4];
	for (int i = 0; i < RESOLUTION; i += 1) {
		for (int j = 0; j < RESOLUTION; j += 1) {
			phaseArray[i * RESOLUTION * 4 + j * 4] = ((float)rand() / RAND_MAX) * 2.0f * M_PI;
			phaseArray[i * RESOLUTION * 4 + j * 4 + 1] = 0.0f;
			phaseArray[i * RESOLUTION * 4 + j * 4 + 2] = 0.0f;
			phaseArray[i * RESOLUTION * 4 + j * 4 + 3] = 0.0f;
		}
	}
	this->pingPhaseTexture = createTextureFromParams(GL_RGBA16F, GL_RGBA, GL_FLOAT, RESOLUTION, RESOLUTION, phaseArray, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);

	this->initialSpectrumFramebuffer = createFramebufferFromTexture(this->initialSpectrumTexture);
	this->pingPhaseFramebuffer = createFramebufferFromTexture(this->pingPhaseTexture);
	this->pongPhaseFramebuffer = createFramebufferFromTexture(this->pongPhaseTexture);
	this->spectrumFramebuffer = createFramebufferFromTexture(this->spectrumTexture);
	this->displacementMapFramebuffer = createFramebufferFromTexture(this->displacementMap);
	this->normalMapFramebuffer = createFramebufferFromTexture(this->normalMap);
	this->pingTransformFramebuffer = createFramebufferFromTexture(this->pingTransformTexture);
	this->pongTransformFramebuffer = createFramebufferFromTexture(this->pongTransformTexture);
}

void ocean::update(float deltaTime) {
	glViewport(0, 0, RESOLUTION, RESOLUTION);
	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(vaoEmpty);

	glBindFramebuffer(GL_FRAMEBUFFER, this->initialSpectrumFramebuffer);
	this->initialSpectrumProgram->use();
	glUniform2fv(this->initialSpectrumProgram->getUniformLocation("u_wind"), 1, this->wind);
	glUniform1f(this->initialSpectrumProgram->getUniformLocation("u_size"), this->size);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	this->phaseProgram->use();
	glBindFramebuffer(GL_FRAMEBUFFER, this->pingPhase ? this->pongPhaseFramebuffer : this->pingPhaseFramebuffer);
	glUniform1i(this->phaseProgram->getUniformLocation("u_phases"), 20);
	glBindTextureUnit(20, this->pingPhase ? this->pingPhaseTexture : this->pongPhaseTexture);
	this->pingPhase = !this->pingPhase;
	glUniform1f(this->phaseProgram->getUniformLocation("u_deltaTime"), deltaTime);
	glUniform1f(this->phaseProgram->getUniformLocation("u_size"), this->size);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	this->spectrumProgram->use();
	glBindFramebuffer(GL_FRAMEBUFFER, this->spectrumFramebuffer);
	glUniform1i(this->spectrumProgram->getUniformLocation("u_phases"), 20);
	glBindTextureUnit(20, this->pingPhase ? this->pingPhaseTexture : this->pongPhaseTexture);
	glUniform1i(this->spectrumProgram->getUniformLocation("u_initialSpectrum"), 21);
	glBindTextureUnit(21, this->initialSpectrumTexture);
	glUniform1f(this->spectrumProgram->getUniformLocation("u_size"), this->size);
	glUniform1f(this->spectrumProgram->getUniformLocation("u_choppiness"), this->choppiness);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glshaderprogram* subtransformProgram = this->horizontalSubtransformProgram;
	this->horizontalSubtransformProgram->use();

	int iterations = log2(RESOLUTION) * 2;
	for(int i = 0; i < iterations; i++) {
		if (i == 0) {
			glBindFramebuffer(GL_FRAMEBUFFER, this->pingTransformFramebuffer);
			glUniform1i(subtransformProgram->getUniformLocation("u_input"), 20);
			glBindTextureUnit(20, this->spectrumTexture);
		} else if (i == iterations - 1) {
			glBindFramebuffer(GL_FRAMEBUFFER, this->displacementMapFramebuffer);
			glUniform1i(subtransformProgram->getUniformLocation("u_input"), 20);
			glBindTextureUnit(20, (iterations % 2 == 0) ? this->pingTransformTexture : this->pongTransformTexture);
		} else if (i % 2 == 1) {
			glBindFramebuffer(GL_FRAMEBUFFER, this->pongTransformFramebuffer);
			glUniform1i(subtransformProgram->getUniformLocation("u_input"), 20);
			glBindTextureUnit(20, this->pingTransformTexture);
		} else {
			glBindFramebuffer(GL_FRAMEBUFFER, this->pingTransformFramebuffer);
			glUniform1i(subtransformProgram->getUniformLocation("u_input"), 20);
			glBindTextureUnit(20, this->pongTransformTexture);
		}

		if (i == iterations / 2) {
			subtransformProgram = this->verticalSubtransformProgram;
			this->verticalSubtransformProgram->use();
		}

		glUniform1f(subtransformProgram->getUniformLocation("u_subtransformSize"), pow(2, (i % (iterations / 2)) + 1));
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, this->normalMapFramebuffer);
	this->normalMapProgram->use();
	glUniform1f(this->normalMapProgram->getUniformLocation("u_size"), this->size);
	glUniform1i(this->normalMapProgram->getUniformLocation("u_displacementMap"), 20);
	glBindTextureUnit(20, this->displacementMap);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glEnable(GL_DEPTH_TEST);
	resetFBO();
}

void ocean::render(glshaderprogram* program) {
	program->use();
	glUniform1f(program->getUniformLocation("geometrySize"), GEOMETRY_SIZE);
	glUniform1f(program->getUniformLocation("size"), this->size);
	glBindVertexArray(this->vaoOcean);
	glDrawElements(GL_TRIANGLES, this->countOfIndicesOcean, GL_UNSIGNED_SHORT, 0);	
}

GLuint ocean::getDisplacementMap() {
	return this->displacementMap;
}

GLuint ocean::getNormalMap() {
	return this->normalMap;
}