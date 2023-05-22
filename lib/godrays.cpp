#include<godrays.h>
#include<iostream>

/***************** Render passes for calculating crepuscular or god rays ******************/
using namespace std;
using namespace vmath;

godrays::godrays(int passWidth, int passHeight)
{
    // create an offscreen framebuffer for occlusion render passes
    glCreateFramebuffers(1, &this->occlusionFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, this->occlusionFbo);

    glCreateRenderbuffers(1, &this->rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, this->rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, passWidth, passHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rboDepth);
    
    glCreateTextures(GL_TEXTURE_2D, 1, &this->texOcclusion);
    glBindTexture(GL_TEXTURE_2D, this->texOcclusion);
    glTextureStorage2D(this->texOcclusion, 1, GL_RGBA8, passWidth, passHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->texOcclusion, 0);

    GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuffer);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throwErr("occlusion framebuffer is incomplete\n");
    }

    float screenQuadCoords[] = {
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };
    glCreateVertexArrays(1, &this->vaoScreenQuad);
    glBindVertexArray(this->vaoScreenQuad);
    glCreateBuffers(1, &this->vboScreenQuad);
    glBindBuffer(GL_ARRAY_BUFFER, this->vboScreenQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadCoords), screenQuadCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    try {
        this->godraysProgram = new glshaderprogram({{"shaders/godrays.vert"}, {"shaders/godrays.frag"}});
        this->colorProgram = new glshaderprogram({{"shaders/color.vert"}, {"shaders/color.frag"}});
    } catch(string errorString) {
        throwErr(errorString);
    }
}

godrays::~godrays() {
    if(godraysProgram) {
        delete godraysProgram;
        godraysProgram = NULL;
    }
    if(colorProgram) {
        delete colorProgram;
        colorProgram = NULL;
    }
    if(this->vboScreenQuad) {
        glDeleteBuffers(1, &this->vboScreenQuad);
        this->vboScreenQuad = 0;
    }
    if(this->vaoScreenQuad) {
        glDeleteBuffers(1, &this->vaoScreenQuad);
        this->vaoScreenQuad = 0;
    }
    if(this->texOcclusion) {
        glDeleteTextures(1, &this->texOcclusion);
        this->texOcclusion = 0;
    }
    if(this->rboDepth) {
        glDeleteRenderbuffers(1, &this->rboDepth);
        this->rboDepth = 0;
    }
    if(this->occlusionFbo) {
        glDeleteFramebuffers(1, &this->occlusionFbo);
        this->occlusionFbo = 0;
    }
}

vec4 godrays::transform(const mat4 &m, const vec4 &v) {
    vec4 out;
    out[0] = m[0][0]*v[0] + m[1][0]*v[1] + m[2][0]*v[2] + m[3][0]*v[3];
    out[1] = m[0][1]*v[0] + m[1][1]*v[1] + m[2][1]*v[2] + m[3][1]*v[3];
    out[2] = m[0][2]*v[0] + m[1][2]*v[1] + m[2][2]*v[2] + m[3][2]*v[3];
    out[3] = m[0][3]*v[0] + m[1][3]*v[1] + m[2][3]*v[2] + m[3][3]*v[3];
    return out;
}

GLuint godrays::getFbo(void) {
    return this->occlusionFbo;
}

void godrays::occlusionPass(const mat4 &mvpMatrix, bool isEmissive) {
    colorProgram->use();
    glUniformMatrix4fv(this->colorProgram->getUniformLocation("mvpMatrix"), 1, GL_FALSE, mvpMatrix);
    if(isEmissive)
        glUniform4fv(this->colorProgram->getUniformLocation("color"), 1, vec4(1.0f, 1.0f, 1.0f, 1.0f));
    else
        glUniform4fv(this->colorProgram->getUniformLocation("color"), 1, vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

void godrays::render(const mat4 &mvpMatrixEmissiveObj, const vec4 &posEmissiveObj, float density, float weight, float decay, float exposure, int samples) {
    // enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // get screen-space emission source coordinates (perspective divide)
    vec4 ssPos = godrays::transform(mvpMatrixEmissiveObj, posEmissiveObj);
    float ssX = ssPos[0]/ssPos[2];
    float ssY = ssPos[1]/ssPos[2];

    // map ssX and ssY from [-1, 1] to [0, 1]
    ssX = ssX*0.5f + 0.5f;
    ssY = ssY*0.5f + 0.5f;

    godraysProgram->use();

    glBindTexture(GL_TEXTURE_2D, this->texOcclusion);
    glActiveTexture(GL_TEXTURE0);

    glUniform1i(0, 0);
    glUniform2f(1, ssX, ssY);
    glUniform1f(2, density);
    glUniform1f(3, weight);
    glUniform1f(4, decay);
    glUniform1f(5, exposure);
    glUniform1i(6, samples);

    glBindVertexArray(this->vaoScreenQuad);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
}

/******************************************************************************************/
