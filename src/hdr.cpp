#include <X11/X.h>
#include<iostream>
#include <iterator>
#include <string>

#include"../include/glshaderloader.h"
#include"../include/gltextureloader.h"
#include"../include/vmath.h"

#include"../include/hdr.h"

using namespace std;

static glshaderprogram* program;
GLuint tempVao;
void setupProgramHDREffect(){
    try {
        program = new glshaderprogram({"src/shaders/hdr.vert", "src/shaders/hdr.frag"});
    } catch (string errorString) {
        cout << errorString<<endl;
    }
}

void initHDREffect(){
    glGenVertexArrays(1,&tempVao);
    glBindVertexArray(tempVao);
}

void renderHDREffect(HDR &hdr)
{
    try {
        program->use();
        glUniform1i(program->getUniformLocation("hdrTex"),hdr.hdrTex);
        glUniform1f(program->getUniformLocation("exposure"),hdr.exposure);
        glUniform1f(program->getUniformLocation("fade"),hdr.fade);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,hdr.hdrTex);
        glBindVertexArray(tempVao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    } catch (string errorString) {
        cout << errorString << endl;
    }
}

void uninitHDREffect()
{
    delete program;
}
