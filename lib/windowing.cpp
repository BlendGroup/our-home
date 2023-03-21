#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>
#include<GL/glew.h>
#include<GL/glx.h>
#include"../include/windowing.h"

glwindow::glwindow() {
	static int frameBufferAttrib[] = {
		GLX_DOUBLEBUFFER, True,
		GLX_X_RENDERABLE, True,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_STENCIL_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		None 
	};
	XSetWindowAttributes winAttribs;
	GLXFBConfig *pGlxFBConfig;
	XVisualInfo *pTempVisInfo = NULL;
	int numFBConfigs;
	int defaultScreen;
	int styleMask;

	this->display = XOpenDisplay(NULL);
	defaultScreen = XDefaultScreen(this->display);
	this->visualInfo = (XVisualInfo *)malloc(sizeof(XVisualInfo));
	pGlxFBConfig = glXChooseFBConfig(this->display, defaultScreen, frameBufferAttrib, &numFBConfigs);
	int bestFrameBufferConfig = -1, worstFrameBufferConfig = -1, bestNumOfSamples = -1, worstNumOfSamples = 999;
	for(int i = 0; i < numFBConfigs; i++) {
		pTempVisInfo = glXGetVisualFromFBConfig(this->display, pGlxFBConfig[i]);
		if(pTempVisInfo != NULL) {
			int samplesBuffer, samples;
			glXGetFBConfigAttrib(this->display, pGlxFBConfig[i], GLX_SAMPLE_BUFFERS, &samplesBuffer);
			glXGetFBConfigAttrib(this->display, pGlxFBConfig[i], GLX_SAMPLES, &samples);
			if(bestFrameBufferConfig < 0 || samplesBuffer && samples > bestNumOfSamples) {
				bestFrameBufferConfig = i;
				bestNumOfSamples = samples;
			} else if(worstFrameBufferConfig < 0 || !samplesBuffer || samplesBuffer < worstNumOfSamples) {
				worstFrameBufferConfig = i;
				worstNumOfSamples = samples;
			}
		}
		XFree(pTempVisInfo);
	}

	this->fbConfig = pGlxFBConfig[bestFrameBufferConfig];
	XFree(pGlxFBConfig);

	this->visualInfo = glXGetVisualFromFBConfig(this->display, this->fbConfig);
	winAttribs.border_pixel = 0;
	winAttribs.background_pixmap = 0;
	winAttribs.colormap = XCreateColormap(this->display, RootWindow(this->display, this->visualInfo->screen), this->visualInfo->visual, AllocNone);
	this->colorMap = winAttribs.colormap;
	winAttribs.background_pixel = BlackPixel(this->display, defaultScreen);
	winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask | StructureNotifyMask;
	styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	this->window = XCreateWindow(this->display, RootWindow(this->display, this->visualInfo->screen), 0, 0, 800, 600, 0, this->visualInfo->depth, InputOutput, this->visualInfo->visual, styleMask, &winAttribs);
	XStoreName(this->display, this->window, "Blend Final");

	Atom windowManagerDelete = XInternAtom(this->display, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(this->display, this->window, &windowManagerDelete, 1);

	XMapWindow(this->display, this->window);

	closed = false;

	typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	glXCreateContextAttribsARBProc glXCreateContextAttribARB = NULL;
	glXCreateContextAttribARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 6,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | GLX_CONTEXT_DEBUG_BIT_ARB,
		None
	};

	this->glxcontext =  glXCreateContextAttribARB(this->display, this->fbConfig, 0, True, attribs);
	glXMakeCurrent(this->display, this->window, this->glxcontext);
	glewInit();
}

void glwindow::processEvents(void) {
	XEvent event;
	KeySym keysym;

	while(XPending(this->display)) {
		XNextEvent(this->display, &event);
		switch(event.type) {
		case KeyPress:
			keysym = XkbKeycodeToKeysym(this->display, event.xkey.keycode, 0, 0);
			break;
		case ConfigureNotify:
			windowSize.width = event.xconfigure.width;
			windowSize.height = event.xconfigure.height;
			break;
		case ClientMessage:
			closed = true;
			break;
		default:
			break;
		}
	}
}

bool glwindow::isClosed(void) {
	return closed;
}

void glwindow::swapBuffers() {
	glXSwapBuffers(this->display, this->window);
}

void glwindow::toggleFullscreen(void) {
	static bool bFullscreen = false;
	Atom wm_state;
	Atom fullscreen;
	XEvent xev = {0};

	wm_state = XInternAtom(this->display, "_NET_WM_STATE", False);
	memset(&xev, 0, sizeof(xev));

	xev.type = ClientMessage;
	xev.xclient.window = this->window;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = bFullscreen ? 0 : 1;
	fullscreen = XInternAtom(this->display, "_NET_WM_STATE_FULLSCREEN", False);
	xev.xclient.data.l[1] = fullscreen;
	XSendEvent(this->display, RootWindow(this->display, this->visualInfo->screen), False, StructureNotifyMask, &xev);
}

glwindow::windowsize_t glwindow::getWindowSize(void) {
	return this->windowSize;
}

glwindow::~glwindow() {
	if(glXGetCurrentContext() == this->glxcontext) {
		glXMakeCurrent(this->display, 0, 0);
	}

	if(this->glxcontext) {
		glXDestroyContext(this->display, this->glxcontext);
	}

	if(this->window) {
		XDestroyWindow(this->display, this->window);
	}

	if(this->colorMap) {
		XFreeColormap(this->display, this->colorMap);
	}

	if(this->visualInfo) {
		free(this->visualInfo);
		this->visualInfo = NULL;
	}

	if(this->display) {
		XCloseDisplay(this->display);
		this->display = NULL;
	}
}
