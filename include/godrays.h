#ifndef __GODRAYS__
#define __GODRAYS__

#include<string>
#include<vmath.h>
#include<glshaderloader.h>
#include<errorlog.h>

class godrays {
private:
    glshaderprogram *godraysProgram;
    glshaderprogram *colorProgram;
    GLuint occlusionFbo;
    GLuint rboDepth;
    GLuint texOcclusion;
    GLuint vaoScreenQuad;
    GLuint vboScreenQuad;

    static vmath::vec4 transform(const vmath::mat4 &m, const vmath::vec4 &v);

public:
    godrays(int passWidth, int passHeight);
    ~godrays();
    GLuint getFbo(void);
    void occlusionPass(const vmath::mat4 &mvpMatrix, bool isEmissive);
    void render(const vmath::mat4 &mvpMatrixEmissiveObj, const vmath::vec4 &posEmissiveObj, float density, float weight, float decay, float exposure, int samples);
};

#endif // __GODRAYS__
