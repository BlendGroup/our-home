#ifndef __MAIN__
#define __MAIN__

void setupProgram(void);
void init(void);
void render(void);
void uninit(void);

struct winParam {
	int w;
	int h;
};
extern struct winParam winSize;

#endif