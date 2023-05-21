#ifndef __GODRAYS__
#define __GODRAYS__

#include<string>
#include<vmath.h>
#include<glshaderloader.h>
#include<errorlog.h>

#define GODRAYS_RENDERPASS_WIDTH  455
#define GODRAYS_RENDERPASS_HEIGHT 256

class godrays {
private:
    glshaderprogram *colorProgram;
    GLuint godraysFbo;
    GLuint defaultFbo;
    GLuint rboDepth;
    GLuint texOcclusion;
    int winWidth, winHeight;
    const vmath::mat4 godraysPerspective = vmath::perspective(
        45.0f,
        (float)GODRAYS_RENDERPASS_WIDTH / (float)GODRAYS_RENDERPASS_HEIGHT,
        0.1f,
        1000.0f
    );

public:
    godrays(GLuint defaultFbo, int defaultWidth, int defaultHeight);
    ~godrays();
    void endOcclusionPass(void);
    void beginOcclusionPass(const vmath::mat4 &mvMatrix, bool isEmissive);
};

#endif // __GODRAYS__
