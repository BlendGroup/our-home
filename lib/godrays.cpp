#include<godrays.h>

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

    try {
        this->colorProgram = new glshaderprogram({{"shaders/color.vert"}, {"shaders/color.frag"}});
    } catch(string errorString) {
        throwErr(errorString);
    }
}

godrays::~godrays() {
    if(this->colorProgram) {
        delete colorProgram;
        colorProgram = NULL;
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

GLuint godrays::getFbo(void) {
    return this->occlusionFbo;
}

void godrays::occlusionPass(const mat4 &mvpMatrix, bool isEmissive) {
    colorProgram->use();
    glUniformMatrix4fv(colorProgram->getUniformLocation("mvpMatrix"), 1, GL_FALSE, mvpMatrix);
    if(isEmissive)
        glUniform4fv(colorProgram->getUniformLocation("color"), 1, vec4(1.0f, 1.0f, 1.0f, 1.0f));
    else
        glUniform4fv(colorProgram->getUniformLocation("color"), 1, vec4(0.0f, 0.0f, 0.0f, 1.0f));
}

void godrays::render(void) {
    // enable additive blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_ALPHA, GL_ONE);

    

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
}

/******************************************************************************************/
