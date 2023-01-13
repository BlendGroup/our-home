#ifndef __MAIN__
#define __MAIN__

#ifdef __cplusplus
#define externdef extern "C"
#else 
#define externdef
#endif
externdef void setupProgram(void);
externdef void init(void);
externdef void render(void);
externdef void uninit(void);

struct winParam {
	int w;
	int h;
};
extern struct winParam winSize;

#endif