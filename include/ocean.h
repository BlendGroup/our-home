#ifndef __OCEAN__
#define __OCEAN__

#include<GL/glew.h>
#include<glshaderloader.h>
#include<vmath.h>

class ocean {
private:
	vmath::vec2 wind;
	int size;
	float choppiness;
	vmath::mat4 mMat;
	
	glshaderprogram* horizontalSubtransformProgram;
	glshaderprogram* verticalSubtransformProgram;
	glshaderprogram* initialSpectrumProgram;
	glshaderprogram* phaseProgram;
	glshaderprogram* spectrumProgram;
	glshaderprogram* normalMapProgram;
	glshaderprogram* oceanProgram;
	GLuint fullscreenVertexBuffer;
	GLuint initialSpectrumFramebuffer;
	GLuint pingPhaseFramebuffer;
	GLuint pongPhaseFramebuffer;
	GLuint spectrumFramebuffer;
	GLuint displacementMapFramebuffer;
	GLuint normalMapFramebuffer;
	GLuint pingTransformFramebuffer;
	GLuint pongTransformFramebuffer;
	bool pingPhase;
	GLuint oceanBuffer;
	GLuint oceanIndexBuffer;
	GLuint countOfIndicesOcean;
	GLuint vaoFullscreen;
	GLuint vaoOcean;
	GLuint initialSpectrumTexture;
	GLuint pingPhaseTexture;
	GLuint pongPhaseTexture;
	GLuint spectrumTexture;
	GLuint displacementMap;
	GLuint normalMap;
	GLuint pingTransformTexture;
	GLuint pongTransformTexture;
public:
	ocean(vmath::mat4& mMat, vmath::vec2& wind, float choppiness, int size);
	void render(void);
};

#endif