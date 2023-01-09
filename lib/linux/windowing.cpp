#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<time.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>
#include<GL/glew.h>
#include<GL/glx.h>
#include<FreeImage.h>

#include"../include/glwindowing.h"

bool isClosed = false;
winParam winSize;

static keyboardfunc keyboardFunction;
static mousefunc mouseFunction;

static Display* gpDisplay;
static XVisualInfo* gpXVisualInfo;
static Window gWindow;
static GLXContext gGlxContex;
static Colormap gColormap;
static uint64_t clockOffset = 0; 
static const clockid_t currentclock = CLOCK_REALTIME;
static const uint64_t frequency = 1000000000;
static glcontext contextProp = { 3, 0, GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB, 0};

uint64_t getTimerValue() {
	struct timespec ts;
	clock_gettime(currentclock, &ts);
	return (uint64_t)ts.tv_sec * frequency + (uint64_t)ts.tv_nsec;
}

void toggleFullscreen(void) {
	static bool dl_bFullscreen = false;
	
	Atom dl_wm_state;
	Atom dl_fullscreen;
	XEvent dl_xev = {0};

	dl_wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
	memset(&dl_xev, 0, sizeof(dl_xev));

	dl_xev.type = ClientMessage;
	dl_xev.xclient.window = gWindow;
	dl_xev.xclient.message_type = dl_wm_state;
	dl_xev.xclient.format = 32;
	dl_xev.xclient.data.l[0] = dl_bFullscreen ? 0 : 1;
	dl_fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	dl_xev.xclient.data.l[1] = dl_fullscreen;
	XSendEvent(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen), False, StructureNotifyMask, &dl_xev);

	dl_bFullscreen = !dl_bFullscreen;
}

void setGLContextVersion(int majorVersion, int minorVersion) {
	contextProp.glMajorVersion = majorVersion;
	contextProp.glMinorVersion = minorVersion;
}

void setGLContextFlags(int flagBits) {
	contextProp.glContextFlags = DL_NO_CONTEXT_BIT;
	if(flagBits & DL_DEBUG_CONTEX_BIT) {
		contextProp.glContextFlags |= GLX_CONTEXT_DEBUG_BIT_ARB;
	}
	if(flagBits & DL_FORWARD_CONTEX_BIT) {
		contextProp.glContextFlags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
	}
}

void setGLContextProfile(int profile) {
	switch(profile) {
	case DL_CORE_CONTEXT:
		contextProp.glContextProfile = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
		break;
	case DL_ES_CONTEXT:
		contextProp.glContextProfile = GLX_CONTEXT_ES_PROFILE_BIT_EXT;
		break;
	case DL_COMPATIBILTY_CONTEXT:
		contextProp.glContextProfile = GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		break;
	}
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n", ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ), type, severity, message);
}

void createOpenGLWindow(void) {
	static int dl_frameBufferAttrib[] = {
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
	XSetWindowAttributes dl_winAttribs;
	GLXFBConfig *pGlxFBConfig;
	XVisualInfo *pTempVisInfo = NULL;
	int numFBConfigs;
	int dl_defaultScreen;
	int dl_styleMask;
	
	clockOffset = getTimerValue();

	gpDisplay = XOpenDisplay(NULL);
	dl_defaultScreen = XDefaultScreen(gpDisplay);
	gpXVisualInfo = (XVisualInfo *)malloc(sizeof(XVisualInfo));

	pGlxFBConfig = glXChooseFBConfig(gpDisplay, dl_defaultScreen, dl_frameBufferAttrib, &numFBConfigs);
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

	GLXFBConfig gGLXFBconfig = pGlxFBConfig[bestFrameBufferConfig];
	XFree(pGlxFBConfig);

	gpXVisualInfo = glXGetVisualFromFBConfig(gpDisplay, gGLXFBconfig);

	dl_winAttribs.border_pixel = 0;
	dl_winAttribs.background_pixmap = 0;
	dl_winAttribs.colormap = XCreateColormap(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen), gpXVisualInfo->visual, AllocNone);
	gColormap = dl_winAttribs.colormap;
	dl_winAttribs.background_pixel = BlackPixel(gpDisplay, dl_defaultScreen);
	dl_winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | Button1MotionMask | PointerMotionMask | StructureNotifyMask;
	dl_styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	gWindow = XCreateWindow(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen), 0, 0, 800, 600, 0, gpXVisualInfo->depth, InputOutput, gpXVisualInfo->visual, dl_styleMask, &dl_winAttribs);

	XStoreName(gpDisplay, gWindow, "OpenGL Programmable Pipeline");

	Atom dl_windowManagerDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(gpDisplay, gWindow, &dl_windowManagerDelete, 1);

	XMapWindow(gpDisplay, gWindow);
	
	typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	glXCreateContextAttribsARBProc glXCreateContextAttribARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, contextProp.glMajorVersion,	
		GLX_CONTEXT_MINOR_VERSION_ARB, contextProp.glMinorVersion,
		GLX_CONTEXT_FLAGS_ARB, contextProp.glContextFlags,
		GLX_CONTEXT_PROFILE_MASK_ARB, contextProp.glContextProfile,
		None	
	};

	gGlxContex =  glXCreateContextAttribARB(gpDisplay, gGLXFBconfig, 0, True, attribs);
	glXMakeCurrent(gpDisplay, gWindow, gGlxContex);
	glewInit();
	glDebugMessageCallback(MessageCallback, 0);
}

void enableGLDebugging() {
	glEnable(GL_DEBUG_OUTPUT);
}

void setKeyboardFunc(keyboardfunc key) {
	keyboardFunction = key;
}

void setMouseFunc(mousefunc mouse) {
	mouseFunction = mouse;
}

void processEvents(void) {
	XEvent dl_event;
	KeySym dl_keysym;
	
	while(XPending(gpDisplay)) {
		XNextEvent(gpDisplay, &dl_event);
		switch(dl_event.type) {
		case KeyPress:
			if(keyboardFunction) {
				KeySym keysym = XkbKeycodeToKeysym(gpDisplay, dl_event.xkey.keycode, 0, 0);
				keyboardFunction(keysym, dl_event.xkey.state);
			}
			break;
		case MotionNotify:
			if(mouseFunction) {
				mouseFunction(dl_event.xmotion.x, dl_event.xmotion.y);
			}
			break;
		case ButtonPress:
			break;
		case ConfigureNotify:
			winSize.w = dl_event.xconfigure.width;
			winSize.h = dl_event.xconfigure.height;
			break;
		case ClientMessage:
			isClosed = true;
			break;
		default:
			break;
		}
	}
}

bool isOpenGLWindowClosed() {
	return isClosed;
}

void swapBuffers(void) {
	glXSwapBuffers(gpDisplay, gWindow);
}

void closeOpenGLWindow(void) {
	XEvent ev;
	memset(&ev, 0, sizeof (ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = gWindow;
	ev.xclient.message_type = XInternAtom(gpDisplay, "WM_PROTOCOLS", true);
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", false);
	ev.xclient.data.l[1] = CurrentTime;

	XSendEvent(gpDisplay, gWindow, False, NoEventMask, &ev);			
}

void destroyOpenGLWindow(void) {
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

double getTime() {
	return (double)(getTimerValue() - clockOffset) / frequency;
}
