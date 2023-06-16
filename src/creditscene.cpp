#include<scenes/credit.h>
#include<glshaderloader.h>
#include<global.h>
#include<vmath.h>
#include<iostream>
#include<eventmanager.h>
#include<crossfade.h>
#include<audio.h>
#include<gltextureloader.h>
#include<debugcamera.h>
#include<X11/keysym.h>

using namespace std;
using namespace vmath;

static glshaderprogram* programRender;
static audioplayer *playerIntro;
static debugCamera* cam;
static GLuint textures[7];
static GLuint emptyVao;
static GLuint current = 0;
static GLfloat fade = 0.0f;

enum tvalues {
	FADEIN1_T,
	WAIT1_T,
	FADEOUT1_T,
	FADEIN2_T,
	WAIT2_T,
	FADEOUT2_T,
	FADEIN3_T,
	WAIT3_T,
	FADEOUT3_T,
	FADEIN4_T,
	WAIT4_T,
	FADEOUT4_T,
	FADEIN5_T,
	WAIT5_T,
	FADEOUT5_T,
	FADEIN6_T,
	WAIT6_T,
	FADEOUT6_T,
	FADEIN7_T,
	WAIT7_T,
	FADEOUT7_T,
};

static eventmanager* creditevents;

void creditscene::setupProgram() {
	programRender = new glshaderprogram({"shaders/fsquad.vert", "shaders/credit/render.frag"});
}

void creditscene::setupCamera() {
	cam = new debugCamera(vec3(0.0f), 0.0f, 0.0f);
}

void creditscene::init() {
	creditevents = new eventmanager({
		{FADEIN1_T, { 0.0f, 2.0f }},
		{WAIT1_T, { 2.0f, 8.0f }},
		{FADEOUT1_T, { 10.0f, 2.0f }},
		
		{FADEIN2_T, { 12.0f, 2.0f }},
		{WAIT2_T, { 14.0f, 8.0f }},
		{FADEOUT2_T, { 22.0f, 2.0f }},
		
		{FADEIN3_T, { 24.0f, 2.0f }},
		{WAIT3_T, { 26.0f, 6.0f }},
		{FADEOUT3_T, { 32.0f, 2.0f }},
		
		{FADEIN4_T, { 34.0f, 2.0f }},
		{WAIT4_T, { 36.0f, 4.0f }},
		{FADEOUT4_T, { 40.0f, 2.0f }},
		
		{FADEIN5_T, { 42.0f, 2.0f }},
		{WAIT5_T, { 44.0f, 4.0f }},
		{FADEOUT5_T, { 48.0f, 2.0f }},
		
		{FADEIN6_T, { 50.0f, 2.0f }},
		{WAIT6_T, { 52.0f, 6.0f }},
		{FADEOUT6_T, { 58.0f, 2.0f }},
		
		{FADEIN7_T, { 60.0f, 2.0f }},
		{WAIT7_T, { 62.0f, 8.0f }},
		{FADEOUT7_T, { 70.0f, 2.0f }},
	});

	string names[] = {
		"tech.png",
		"members.png",
		"leaders.png",
		"guide.png",
		"guide2.png",
		"references.png",
		"thanks.png"
	};
	for(int i = 0; i < 7; i++) {
		textures[i] = createTexture2D("resources/textures/credit/" + names[i], GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
	}
	glGenVertexArrays(1, &emptyVao);
	playerIntro = new audioplayer("resources/audio/Intro.wav");
}

void creditscene::render() {
}

void creditscene::update(void) {
	creditevents->increment();
	if(creditevents->getT() > 0.00001f && creditevents->getT() <= 10.0f) {
		playerIntro->play();
	}
	if((*creditevents)[current * 3] > 0.0f && (*creditevents)[current * 3] < 1.0f) {
		fade = (*creditevents)[current * 3];
	} else if((*creditevents)[current * 3 + 2] > 0.0f && (*creditevents)[current * 3 + 2] < 1.0f) {
		fade = 1.0f - (*creditevents)[current * 3 + 2];
	}
	if((*creditevents)[current * 3 + 2] >= 1.0f) {
		current++;
		cout<<current<<endl;
		if(current == 7) {
			playNextScene();
		}
	}
}

void creditscene::reset(void) {
	creditevents->resetT();
}

void creditscene::uninit() {
	delete programRender;
}

void creditscene::keyboardfunc(int key) {
	switch(key) {
	case XK_Up:
		(*creditevents) += 0.4f;
		break;
	case XK_Down:
		(*creditevents) -= 0.4f;
		break;
	}
}

camera* creditscene::getCamera() {
	return cam;
}

void creditscene::crossfade() {
	programRender->use();
	glBindVertexArray(emptyVao);
	glUniform1i(programRender->getUniformLocation("tex"), 0);
	glUniform1f(programRender->getUniformLocation("fade"), fade);
	glBindTextureUnit(0, textures[current]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}