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
	samples(0),
	occlusionTex(0) {
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

vec4 transform(const mat4 &m, const vec4 &v) {
	vec4 out;
	out[0] = m[0][0]*v[0] + m[1][0]*v[1] + m[2][0]*v[2] + m[3][0]*v[3];
	out[1] = m[0][1]*v[0] + m[1][1]*v[1] + m[2][1]*v[2] + m[3][1]*v[3];
	out[2] = m[0][2]*v[0] + m[1][2]*v[1] + m[2][2]*v[2] + m[3][2]*v[3];
	out[3] = m[0][3]*v[0] + m[1][3]*v[1] + m[2][3]*v[2] + m[3][3]*v[3];
	return out;
}

void godrays::renderRays() {
	// enable blending and set blend func to perform additive blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	// get screen-space emission source coordinates (perspective divide)
	vec4 ssPos = transform(this->mvpMatrix, this->pos);
	float ssX = ssPos[0]/ssPos[2];
	float ssY = ssPos[1]/ssPos[2];

	// map ssX and ssY from [-1, 1] to [0, 1]
	ssX = ssX * 0.5f + 0.5f;
	ssY = ssY * 0.5f + 0.5f;

	godraysProgram->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, occlusionTex);

	glUniform1i(godraysProgram->getUniformLocation("texOcclusion"), 0);
	glUniform2fv(godraysProgram->getUniformLocation("screenSpaceEmissiveObjectPos"), 1, sscoord);
	glUniform1f(godraysProgram->getUniformLocation("density"), density);
	glUniform1f(godraysProgram->getUniformLocation("weight"), weight);
	glUniform1f(godraysProgram->getUniformLocation("decay"), decay);
	glUniform1f(godraysProgram->getUniformLocation("exposure"), exposure);
	glUniform1i(godraysProgram->getUniformLocation("samples"), samples);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// reset blend func to default and disable blending to prevent further BT of life
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
}

/******************************************************************************************/
