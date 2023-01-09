#ifndef __WINDOWING__
#define __WINDOWING__

#define DL_NO_CONTEXT_BIT 0x0000
#define DL_DEBUG_CONTEX_BIT 0x0002
#define DL_FORWARD_CONTEX_BIT 0x0001
#define DL_CORE_CONTEXT 100
#define DL_ES_CONTEXT 101
#define DL_COMPATIBILTY_CONTEXT 102

#if defined(__gnu_linux__) || defined(__linux__)
	#include<X11/keysym.h>
	#define DL_ESCAPE XK_Escape
	#define DL_LEFT XK_Left
	#define DL_RIGHT XK_Right
	#define DL_UP XK_Up
	#define DL_DOWN XK_Down
#elif defined(_win32) || defined(_win64)
	#include<Windows.h>
	#define DL_ESCAPE VK_ESCAPE
#endif

typedef struct winParam_t {
	float w;
	float h;
} winParam;

typedef struct glcontext_t {
	int glMajorVersion;
	int glMinorVersion;
	int	glContextProfile;
	int glContextFlags;
} glcontext;

extern winParam winSize;

typedef void (*keyboardfunc)(unsigned int key, int state);
typedef void (*mousefunc)(int x, int y);

void toggleFullscreen(void);
void setGLContextVersion(int majorVersion, int minorVersion);
void setGLContextFlags(int flagBits);
void setGLContextProfile(int profile);
void createOpenGLWindow(void);
void processEvents(void);
void closeOpenGLWindow(void);
void destroyOpenGLWindow(void);
bool isOpenGLWindowClosed();
void swapBuffers(void);
void setKeyboardFunc(keyboardfunc key);
void setMouseFunc(mousefunc mouse);
void enableGLDebugging();
double getTime();

#endif