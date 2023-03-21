#ifndef __WINDOWING__
#define __WINDOWING__

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<GL/glx.h>

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
public:
	glwindow();
	void toggleFullscreen(void);
	void processEvents(void);
	bool isClosed(void);
	windowsize_t getWindowSize();
	void swapBuffers(void);
	~glwindow();
};

#endif