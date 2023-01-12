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

Display *dl_gpDisplay = NULL;
XVisualInfo *dl_gpXVisualInfo = NULL;
Colormap dl_gColormap;
Window dl_gWindow;
GLXContext dl_gGlxContex;
GLXFBConfig gGLXFBconfig;

GLuint vertShaderObj;
GLuint fragShaderObj;
GLuint programObj;

typedef GLXContext(*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribARB = NULL;

int dl_giWindowWidth = 800;
int dl_giWindowHeight = 600;

int main(void) {
	void createWindow(void);
	void toggleFullscreen(void);
	void initWindow(void);
	void uninitWindow(void);

	bool dl_bDone = false;

	createWindow();
	initWindow();
	toggleFullscreen();

	XEvent dl_event;
	KeySym dl_keysym;

	while(!dl_bDone) {
		while(XPending(dl_gpDisplay)) {
			XNextEvent(dl_gpDisplay, &dl_event);
			switch(dl_event.type) {
			case KeyPress:
				dl_keysym = XkbKeycodeToKeysym(dl_gpDisplay, dl_event.xkey.keycode, 0, 0);
				switch(dl_keysym) {
				case XK_Escape:
					dl_bDone = true;
					break;
				case XK_F: case XK_f:
					toggleFullscreen();
					break;
				default:
					break;
				}
				break;
			case ConfigureNotify:
				winSize.w = dl_event.xconfigure.width;
				winSize.h = dl_event.xconfigure.height;
				break;
			case ClientMessage:
				dl_bDone = true;
				break;
			default:
				break;
			}
		}
		render();
		glXSwapBuffers(dl_gpDisplay, dl_gWindow);
	}
	uninitWindow();
	return 0;
}

void createWindow(void) {
	void uninitWindow(void);

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

	dl_gpDisplay = XOpenDisplay(NULL);
	if(dl_gpDisplay == NULL) {
		printf("ERROR");
		uninitWindow();
		exit(1);
	}

	dl_defaultScreen = XDefaultScreen(dl_gpDisplay);

	dl_gpXVisualInfo = (XVisualInfo *)malloc(sizeof(XVisualInfo));

	if(dl_gpXVisualInfo == NULL) {
		printf("ERROR");
		uninitWindow();
		exit(1);
	}

	pGlxFBConfig = glXChooseFBConfig(dl_gpDisplay, dl_defaultScreen, dl_frameBufferAttrib, &numFBConfigs);
	printf("Number Of FBConfigs Found = %d\n", numFBConfigs);
	int bestFrameBufferConfig = -1, worstFrameBufferConfig = -1, bestNumOfSamples = -1, worstNumOfSamples = 999;
	for(int i = 0; i < numFBConfigs; i++) {
		pTempVisInfo = glXGetVisualFromFBConfig(dl_gpDisplay, pGlxFBConfig[i]);
		if(pTempVisInfo != NULL) {
			int samplesBuffer, samples;
			glXGetFBConfigAttrib(dl_gpDisplay, pGlxFBConfig[i], GLX_SAMPLE_BUFFERS, &samplesBuffer);
			glXGetFBConfigAttrib(dl_gpDisplay, pGlxFBConfig[i], GLX_SAMPLES, &samples);
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

	dl_gpXVisualInfo = glXGetVisualFromFBConfig(dl_gpDisplay, gGLXFBconfig);

	if(dl_gpXVisualInfo == NULL) {
		printf("ERROR");
		uninitWindow();
		exit(1);
	}

	dl_winAttribs.border_pixel = 0;
	dl_winAttribs.background_pixmap = 0;
	dl_winAttribs.colormap = XCreateColormap(dl_gpDisplay, RootWindow(dl_gpDisplay, dl_gpXVisualInfo->screen), dl_gpXVisualInfo->visual, AllocNone);
	dl_gColormap = dl_winAttribs.colormap;
	dl_winAttribs.background_pixel = BlackPixel(dl_gpDisplay, dl_defaultScreen);
	dl_winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask | StructureNotifyMask;
	dl_styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	dl_gWindow = XCreateWindow(dl_gpDisplay, RootWindow(dl_gpDisplay, dl_gpXVisualInfo->screen), 0, 0, dl_giWindowWidth, dl_giWindowHeight, 0, dl_gpXVisualInfo->depth, InputOutput, dl_gpXVisualInfo->visual, dl_styleMask, &dl_winAttribs);

	if(!dl_gWindow) {
		printf("ERROR");
	}

	XStoreName(dl_gpDisplay, dl_gWindow, "OpenGL Programmable Pipeline");

	Atom dl_windowManagerDelete = XInternAtom(dl_gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(dl_gpDisplay, dl_gWindow, &dl_windowManagerDelete, 1);

	XMapWindow(dl_gpDisplay, dl_gWindow);
}

void toggleFullscreen(void) {
	static bool dl_bFullscreen = false;
	Atom dl_wm_state;
	Atom dl_fullscreen;
	XEvent dl_xev = {0};

	dl_wm_state = XInternAtom(dl_gpDisplay, "_NET_WM_STATE", False);
	memset(&dl_xev, 0, sizeof(dl_xev));

	dl_xev.type = ClientMessage;
	dl_xev.xclient.window = dl_gWindow;
	dl_xev.xclient.message_type = dl_wm_state;
	dl_xev.xclient.format = 32;
	dl_xev.xclient.data.l[0] = dl_bFullscreen ? 0 : 1;
	dl_fullscreen = XInternAtom(dl_gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	dl_xev.xclient.data.l[1] = dl_fullscreen;
	XSendEvent(dl_gpDisplay, RootWindow(dl_gpDisplay, dl_gpXVisualInfo->screen), False, StructureNotifyMask, &dl_xev);
}

void initWindow() {
	void uninitWindow(void);
	void Resize(int, int);

	glXCreateContextAttribARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	if(!glXCreateContextAttribARB) {
		printf("ERROR: Function Not Found\n");
		uninitWindow();
		exit(1);
	}

	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 6,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	dl_gGlxContex =  glXCreateContextAttribARB(dl_gpDisplay, gGLXFBconfig, 0, True, attribs);

	if(!dl_gGlxContex) {
		const int attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		};
		dl_gGlxContex =  glXCreateContextAttribARB(dl_gpDisplay, gGLXFBconfig, 0, True, attribs);
		printf("Couldn't Find OpenGL 4.6 defaulting to a lower version.\n");
	} else {
		printf("Found OpenGL 4.6 Supporting Context\n"); 
	}

	if(glXIsDirect(dl_gpDisplay, dl_gGlxContex)) {
		printf("GLXContext supports Hardware Rendering\n");
	} else {
		printf("GLXContext doesn't support Hardware Rendering, defaulting to Software Rendering\n");
	}

	glXMakeCurrent(dl_gpDisplay, dl_gWindow, dl_gGlxContex);

	if(glewInit() != GLEW_NO_ERROR) {
		printf("GLEW Init Failed\n");
		uninitWindow();
		exit(1);
	}

	init();
}

void uninitWindow(void) {
	GLXContext dl_currentContext;

	dl_currentContext = glXGetCurrentContext();

	uninit();

	if(dl_currentContext == dl_gGlxContex) {
		glXMakeCurrent(dl_gpDisplay, 0, 0);
	}

	if(dl_gGlxContex) {
		glXDestroyContext(dl_gpDisplay, dl_gGlxContex);
	}

	if(dl_gWindow) {
		XDestroyWindow(dl_gpDisplay, dl_gWindow);
	}

	if(dl_gColormap) {
		XFreeColormap(dl_gpDisplay, dl_gColormap);
	}

	if(dl_gpXVisualInfo) {
		free(dl_gpXVisualInfo);
		dl_gpXVisualInfo = NULL;
	}

	if(dl_gpDisplay) {
		XCloseDisplay(dl_gpDisplay);
		dl_gpDisplay = NULL;
	}
}
