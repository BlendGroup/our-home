#ifndef __WINDOWING__
#define __WINDOWING__

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<GL/glx.h>
#include<iostream>

enum MOUSE {
	MOUSE_BUTTON_PRESS,
	MOUSE_BUTTON_RELEASE,
	MOUSE_BUTTON_MOVE
};

class glwindow;

typedef void (*glwindowkeyboardfunc)(glwindow* window, int key);
typedef void (*glwindowmousefunc)(glwindow* window, int button, int action, int x, int y);

class glwindow {
private:
	Display* display;
	XVisualInfo* visualInfo;
	Colormap colorMap;
	GLXFBConfig fbConfig;
	Window window;
	GLXContext glxcontext;
	bool closed;
	bool fullscreen;
	struct windowsize_t {
		float width;
		float height;
	} windowSize;
	glwindowkeyboardfunc keyboardFunc;
	glwindowmousefunc mouseFunc;
public:
	glwindow(std::string name, int x, int y, int width, int height, int glversion);
	void toggleFullscreen(void);
	void setFullscreen(bool fullscreen);
	void processEvents(void);
	bool isClosed(void);
	void close(void);
	windowsize_t getSize(void);
	void swapBuffers(void);
	void setKeyboardFunc(glwindowkeyboardfunc keyboardcallback);
	void setMouseFunc(glwindowmousefunc mousecallback);
	~glwindow(void);
};

#endif