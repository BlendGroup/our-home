#ifndef __WINDOWING__
#define __WINDOWING__

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<GL/glx.h>
#include<iostream>

class glwindow;

typedef void (*glwindowkeyboardfunc)(glwindow* window, int key);

class glwindow {
private:
	Display* display;
	XVisualInfo* visualInfo;
	Colormap colorMap;
	GLXFBConfig fbConfig;
	Window window;
	GLXContext glxcontext;
	bool closed;
	struct windowsize_t {
		float width;
		float height;
	} windowSize;
	glwindowkeyboardfunc keyboardFunc;
public:
	glwindow(std::string name, int x, int y, int width, int height, int glversion);
	void toggleFullscreen(void);
	void processEvents(void);
	bool isClosed(void);
	void closeWindow(void);
	windowsize_t getWindowSize(void);
	void swapBuffers(void);
	void setKeyboardFunc(glwindowkeyboardfunc keyboardcallback);
	~glwindow(void);
};

#endif