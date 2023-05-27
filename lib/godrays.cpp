#include<godrays.h>
#include<iostream>

/***************** Render passes for calculating crepuscular or god rays ******************/
using namespace std;
using namespace vmath;

godrays::godrays():
	sscoord(vec2(0.0f, 0.0f)),
	density(0.0f),
	weight(0.0f),
	decay(0.0f),
	exposure(0.0f),
	samples(0) {
	glGenVertexArrays(1, &emptyvao);
	try {
		godraysProgram = new glshaderprogram({{"shaders/godrays.vert"}, {"shaders/godrays.frag"}});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

godrays::~godrays() {
	if(godraysProgram) {
		delete godraysProgram;
		godraysProgram = NULL;
	}
}

void godrays::setDensity(GLfloat density) {
	this->density = density;
}

void godrays::setWeight(GLfloat weight) {
	this->weight = weight;
}

void godrays::setDecay(GLfloat decay) {
	this->decay = decay;
}

void godrays::setExposure(GLfloat exposure) {
	this->exposure = exposure;
}

void godrays::setSamples(int samples) {
	this->samples = samples;
}

vec4 transform(const mat4 &m, const vec4 &v) {
	vec4 out;
	out[0] = m[0][0]*v[0] + m[1][0]*v[1] + m[2][0]*v[2] + m[3][0]*v[3];
	out[1] = m[0][1]*v[0] + m[1][1]*v[1] + m[2][1]*v[2] + m[3][1]*v[3];
	out[2] = m[0][2]*v[0] + m[1][2]*v[1] + m[2][2]*v[2] + m[3][2]*v[3];
	out[3] = m[0][3]*v[0] + m[1][3]*v[1] + m[2][3]*v[2] + m[3][3]*v[3];
	return out;
}

void godrays::setScreenSpaceCoords(const vmath::mat4& mvp, const vmath::vec4& pos) {
	vec4 ssPos = transform(mvp, pos);
	this->sscoord = vec2(ssPos[0] / ssPos[2] * 0.5f + 0.5f, ssPos[1] / ssPos[2] * 0.5f + 0.5f);
}

void godrays::renderRays(HDR* hdr) {
	// enable blending and set blend func to perform additive blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	godraysProgram->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdr->occlusionTex);

	glUniform1i(godraysProgram->getUniformLocation("texOcclusion"), 0);
	glUniform2fv(godraysProgram->getUniformLocation("screenSpaceEmissiveObjectPos"), 1, vec2(0.5f, 0.5f));
	glUniform1f(godraysProgram->getUniformLocation("density"), density);
	glUniform1f(godraysProgram->getUniformLocation("weight"), weight);
	glUniform1f(godraysProgram->getUniformLocation("decay"), decay);
	glUniform1f(godraysProgram->getUniformLocation("exposure"), exposure);
	glUniform1i(godraysProgram->getUniformLocation("samples"), samples);

	glBindVertexArray(this->emptyvao);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// reset blend func to default and disable blending to prevent further BT of life
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
}

/******************************************************************************************/
