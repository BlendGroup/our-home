#include<modelplacer.h>
#include<vmath.h>
#include<X11/keysym.h>

using namespace std;
using namespace vmath;

modelplacer::modelplacer() {
	this->multiplier = 1.0f;
	this->rotateBy = vec3(0.0f, 0.0f, 0.0f);
	this->translateBy = vec3(0.0f, 0.0f, 0.0f);
	this->scaleBy = 1.0f;
}

modelplacer::modelplacer(vec3 trans, vec3 rot, float sca) {
	this->multiplier = 1.0f;
	this->translateBy = trans;
	this->rotateBy = rot;
	this->scaleBy = sca;
}

ostream& operator<<(ostream& out, modelplacer* modelplacer) {
	out<<"To load: "<<modelplacer->translateBy<<", "<<modelplacer->rotateBy<<", "<<modelplacer->scaleBy<<"f\n";
	out<<"To use : translate("<<modelplacer->translateBy[0]<<"f, "<<modelplacer->translateBy[1]<<"f, "<<modelplacer->translateBy[2]<<"f) * ";
	if(modelplacer->rotateBy[0] != 0.0f) {
		out<<"rotate("<<modelplacer->rotateBy[0]<<"f, 1.0f, 0.0f, 0.0f) * ";
	}
	if(modelplacer->rotateBy[1] != 0.0f) {
		out<<"rotate("<<modelplacer->rotateBy[1]<<"f, 0.0f, 1.0f, 0.0f) * ";
	}
	if(modelplacer->rotateBy[2] != 0.0f) {
		out<<"rotate("<<modelplacer->rotateBy[2]<<"f, 0.0f, 0.0f, 1.0f) * ";
	}
	out<<"scale("<<modelplacer->scaleBy<<"f);";
	return out;
}

mat4 modelplacer::getModelMatrix(void) {
	return translate(this->translateBy) * rotate(this->rotateBy[0], 1.0f, 0.0f, 0.0f) * rotate(this->rotateBy[1], 0.0f, 1.0f, 0.0f) * rotate(this->rotateBy[2], 0.0f, 0.0f, 1.0f) * scale(this->scaleBy);
}

void modelplacer::keyboardfunc(int key) {
	switch(key) {
	case XK_i:
		this->translateBy[2] -= this->multiplier;
		break;
	case XK_k:
		this->translateBy[2] += this->multiplier;
		break;
	case XK_j:
		this->translateBy[0] -= this->multiplier;
		break;
	case XK_l:
		this->translateBy[0] += this->multiplier;
		break;
	case XK_u:
		this->translateBy[1] -= this->multiplier;
		break;
	case XK_o:
		this->translateBy[1] += this->multiplier;
		break;

	case XK_t:
		this->rotateBy[2] -= 1.0f;
		break;
	case XK_g:
		this->rotateBy[2] += 1.0f;
		break;
	case XK_f:
		this->rotateBy[0] -= 1.0f;
		break;
	case XK_h:
		this->rotateBy[0] += 1.0f;
		break;
	case XK_r:
		this->rotateBy[1] -= 1.0f;
		break;
	case XK_y:
		this->rotateBy[1] += 1.0f;
		break;

	case XK_m:
		this->scaleBy += this->multiplier;
		break;
	case XK_n:
		this->scaleBy -= this->multiplier;
		break;

	case XK_comma:
		this->multiplier /= 10.0f;
		break;
	case XK_period:
		this->multiplier *= 10.0f;
		break;
	}

}