#include <vmath.h>
#include <string>
#include <errorlog.h>
#include <CubeMapRenderTarget.h>

using namespace std;
using namespace vmath;

CubeMapRenderTarget::CubeMapRenderTarget(GLuint width,GLuint height,bool mipLevels){
    
    this->width = width;
    this->height = height;

    glGenTextures(1, &this->cubemap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP,this->cubemap_texture);

    for(size_t i = 0; i < 6; i++){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipLevels ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    if(mipLevels) glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glGenFramebuffers(1,&this->FBO);
    glBindFramebuffer(GL_FRAMEBUFFER,this->FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X,cubemap_texture,0);

    glGenRenderbuffers(1,&this->RBO);
    glBindRenderbuffer(GL_RENDERBUFFER,this->RBO);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,width,height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->RBO);

    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, buffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throwErr("HDR Framebuffer Not Complete !!!");
	}

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glBindRenderbuffer(GL_RENDERBUFFER,0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void CubeMapRenderTarget::setPosition(const vmath::vec3 pos){

    this->position = pos;

    view[0] = lookat(pos, pos + vec3(1,0,0), vec3(0,1,0));
    view[1] = lookat(pos, pos + vec3(-1,0,0), vec3(0,1,0));
    view[2] = lookat(pos, pos + vec3(0,-1,0), vec3(0,0,-1));
    view[3] = lookat(pos, pos + vec3(0,1,0), vec3(0,0,1));
    view[4] = lookat(pos, pos + vec3(0,0,-1), vec3(0,1,0));
    view[5] = lookat(pos, pos + vec3(0,0,1), vec3(0,1,0));

    projection = perspective(90.0f, 1.0f, 0.1f, 200.0f);
}

const vec3 CubeMapRenderTarget::getPosition(void){
    return this->position;
}

CubeMapRenderTarget::~CubeMapRenderTarget(){

    if(this->cubemap_texture){
        glDeleteTextures(1, &this->cubemap_texture);
    }

    if(this->FBO){
        glDeleteFramebuffers(1,&this->FBO);
    }

    if(this->RBO){
        glDeleteRenderbuffers(1,&this->RBO);
    }
}
