#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>
#include<GL/glew.h>
#include<GL/glx.h>

#include"../../include/main.h"

Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;
GLXContext gGlxContex;
GLXFBConfig gGLXFBconfig;

GLuint vertShaderObj;
GLuint fragShaderObj;
GLuint programObj;

typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribARB = NULL;

int main(void) {
	void createWindow(void);
	void toggleFullscreen(void);
	void initWindow(void);
	void uninitWindow(void);

	bool bDone = false;

	createWindow();
	initWindow();
	toggleFullscreen();

	XEvent event;
	KeySym keysym;

	while(!bDone) {
		while(XPending(gpDisplay)) {
			XNextEvent(gpDisplay, &event);
			switch(event.type) {
			case KeyPress:
				keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
				switch(keysym) {
				case XK_Escape:
					bDone = true;
					break;
				case XK_F: case XK_f:
					toggleFullscreen();
					break;
				default:
					break;
				}
				break;
			case ConfigureNotify:
				winSize.w = event.xconfigure.width;
				winSize.h = event.xconfigure.height;
				break;
			case ClientMessage:
				bDone = true;
				break;
			default:
				break;
			}
		}
		render();
		glXSwapBuffers(gpDisplay, gWindow);
	}
	uninitWindow();
	return 0;
}

void createWindow(void) {
	void uninitWindow(void);

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

	gpDisplay = XOpenDisplay(NULL);
	defaultScreen = XDefaultScreen(gpDisplay);
	gpXVisualInfo = (XVisualInfo *)malloc(sizeof(XVisualInfo));
	pGlxFBConfig = glXChooseFBConfig(gpDisplay, defaultScreen, frameBufferAttrib, &numFBConfigs);
	int bestFrameBufferConfig = -1, worstFrameBufferConfig = -1, bestNumOfSamples = -1, worstNumOfSamples = 999;
	for(int i = 0; i < numFBConfigs; i++) {
		pTempVisInfo = glXGetVisualFromFBConfig(gpDisplay, pGlxFBConfig[i]);
		if(pTempVisInfo != NULL) {
			int samplesBuffer, samples;
			glXGetFBConfigAttrib(gpDisplay, pGlxFBConfig[i], GLX_SAMPLE_BUFFERS, &samplesBuffer);
			glXGetFBConfigAttrib(gpDisplay, pGlxFBConfig[i], GLX_SAMPLES, &samples);
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

	gGLXFBconfig = pGlxFBConfig[bestFrameBufferConfig];
	XFree(pGlxFBConfig);

	gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, gGLXFBconfig);
	winAttribs.border_pixel = 0;
	winAttribs.background_pixmap = 0;
	winAttribs.colormap = XCreateColormap(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen), gpXVisualInfo->visual, AllocNone);
	gColormap = winAttribs.colormap;
	winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen);
	winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask | StructureNotifyMask;
	styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	gWindow = XCreateWindow(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen), 0, 0, 800, 600, 0, gpXVisualInfo->depth, InputOutput, gpXVisualInfo->visual, styleMask, &winAttribs);
	XStoreName(gpDisplay, gWindow, "Blend Final");

	Atom windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);

	XMapWindow(gpDisplay, gWindow);
}

void toggleFullscreen(void) {
	static bool bFullscreen = false;
	Atom wm_state;
	Atom fullscreen;
	XEvent xev = {0};

	wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
	memset(&xev, 0, sizeof(xev));

	xev.type = ClientMessage;
	xev.xclient.window = gWindow;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = bFullscreen ? 0 : 1;
	fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xev.xclient.data.l[1] = fullscreen;
	XSendEvent(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen), False, StructureNotifyMask, &xev);
}

void initWindow() {
	void uninitWindow(void);
	void Resize(int, int);

	glXCreateContextAttribARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 6,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | GLX_CONTEXT_DEBUG_BIT_ARB,
		None
	};

	gGlxContex =  glXCreateContextAttribARB(gpDisplay, gGLXFBconfig, 0, True, attribs);
	glXMakeCurrent(gpDisplay, gWindow, gGlxContex);
	glewInit();
	setupProgram();
	init();
}

void uninitWindow(void) {
	GLXContext currentContext;

	currentContext = glXGetCurrentContext();

	uninit();

	if(currentContext == gGlxContex) {
		glXMakeCurrent(gpDisplay, 0, 0);
	}

	if(gGlxContex) {
		glXDestroyContext(gpDisplay, gGlxContex);
	}

	if(gWindow) {
		XDestroyWindow(gpDisplay, gWindow);
	}

	if(gColormap) {
		XFreeColormap(gpDisplay, gColormap);
	}

	if(gpXVisualInfo) {
		free(gpXVisualInfo);
		gpXVisualInfo = NULL;
	}

	if(gpDisplay) {
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
	}
}
