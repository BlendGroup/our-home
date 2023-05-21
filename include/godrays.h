#ifndef __GODRAYS__
#define __GODRAYS__

#include<string>
#include<vmath.h>
#include<glshaderloader.h>
#include<errorlog.h>

class godrays {
private:
    glshaderprogram *colorProgram;
    GLuint occlusionFbo;
    GLuint rboDepth;
    GLuint texOcclusion;

public:
    godrays(int passWidth, int passHeight);
    ~godrays();
    GLuint getFbo(void);
    void occlusionPass(const vmath::mat4 &mvpMatrix, bool isEmissive);
    void render(void);
};

#endif // __GODRAYS__
