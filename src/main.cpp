#include <X11/Xlib.h>
#include <cstddef>
#include<iostream>

#include<GL/glew.h>
#include<GL/gl.h>

#include"../include/vmath.h"
#include"../include/glshaderloader.h"
#include"../include/testeffect.h"
#include "../include/hdr.h"
#include"../include/windowing.h"

using namespace std;
using namespace vmath;

Bool ghdr_enabled = False;
HDR g_hdr;

void setupProgram(void) {
	setupProgramTestEffect();
	setupProgramHDREffect();
}

void init(void) {
	
	glGenFramebuffers(1,&g_hdr.hdrFBO);
	glGenTextures(1, &g_hdr.hdrTex);
	glBindTexture(GL_TEXTURE_2D,g_hdr.hdrTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 2048, 2048, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenRenderbuffers(1,&g_hdr.hdrRBO);
	glBindRenderbuffer(GL_RENDERBUFFER,g_hdr.hdrRBO);
	glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT,2048,2048);

	glBindFramebuffer(GL_FRAMEBUFFER,g_hdr.hdrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,g_hdr.hdrTex,0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,g_hdr.hdrRBO);
	const GLenum buffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1,buffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout<<"HDR Framebuffer Not Complete !!!"<<endl;

	glBindFramebuffer(GL_FRAMEBUFFER,0);

	g_hdr.fade = 1.0f;
	g_hdr.exposure = 5.0f;
	
	initTestEffect();
	initHDREffect();

}

void render(glwindow* window) {
	if(ghdr_enabled)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,g_hdr.hdrFBO);
		glViewport(0, 0, 2048, 2048);
	}
	else 
	{
		glViewport(0, 0, window->getSize().width, window->getSize().height);
	}
	glClearBufferfv(GL_COLOR, 0, vec4(0.5f, 1.0f, 0.2f, 1.0f));
	renderTestEffect();

	if(ghdr_enabled)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,0);
		glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.1f, 0.1f, 1.0f));
		glViewport(0, 0, window->getSize().width, window->getSize().height);
		renderHDREffect(g_hdr);
	}
}

void keyboard(glwindow* window, int key) {
	switch(key) {
	case XK_Escape:
		window->close();
		break;
	case XK_space:
		ghdr_enabled = !ghdr_enabled;
	break;
	case XK_e:
		g_hdr.exposure -= 0.01f;
		cout << g_hdr.exposure << endl;
	break;
	case XK_r:
		g_hdr.exposure += 0.01f;
		cout << g_hdr.exposure << endl;
	break;
	}
}

void uninit(void) {

	uninitTestEffect();
	uninitHDREffect();
}

int main(int argc, char **argv) {
	glwindow* window = new glwindow("Our Planet", 0, 0, 1920, 1080, 460);
	init();
	setupProgram();
	window->setKeyboardFunc(keyboard);
	window->toggleFullscreen();
	while(!window->isClosed()) {
		window->processEvents();
		render(window);
		window->swapBuffers();
	}
	uninit();
	delete window;
	return 0;
}