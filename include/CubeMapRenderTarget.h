#ifndef __CUBEMAP_RT__
#define __CUBEMAP_RT__

#include<vmath.h>
#include<glbase.h>
#include<GL/glew.h>
#include<camera.h>

class CubeMapRenderTarget{
    
    public:
    vmath::mat4 view[6];
    vmath::mat4 projection;
    GLuint cubemap_texture;
    GLuint RBO;
    vmath::vec3 position;
    GLuint FBO;
    GLuint width,height;
    CubeMapRenderTarget(GLuint width,GLuint height,bool mipLevels);
    void setPosition(const vmath::vec3 pos);
    const vmath::vec3 getPosition(void);
    void bindRenderTarget();
    void bindTexture(GLuint unit = 0);
    ~CubeMapRenderTarget();
};

#endif