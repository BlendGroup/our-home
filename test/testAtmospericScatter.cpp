#include <assimp/postprocess.h>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>
#include <vmath.h>

#include <glshaderloader.h>
#include <gltextureloader.h>
#include <glmodelloader.h>
#include <vmath.h>
#include <errorlog.h>
#include <testAtmospericScatter.h>
#include <global.h>

using namespace std;
using namespace vmath;

static glshaderprogram* sky;
static glshaderprogram* SunDepthTest;
static glshaderprogram* BlurH;
static glshaderprogram* BlurV;
static glshaderprogram* SunRaysLenseFlareHalo;
static GLuint dirtTexture,vao,vbo;
static GLuint fbo,rbo,DepthTexture,SunTextures[4];

float SunR;
vec3 SunCPos, SunRotVec;

float Kr = 0.0030f;
float Km = 0.0015f;
float ESun = 16.0f;
float g = -0.75f;
float InnerRadius = 10.0f;
float OuterRadius = 10.25f;
float Scale = 1.0f / (OuterRadius - InnerRadius);
float ScaleDepth = 0.25f;
float ScaleOverScaleDepth = Scale / ScaleDepth;

void setupProgramAS(){
    try 
    {
        sky = new glshaderprogram({"shaders/AtmosphericScattering/skyfromatmosphere.vert","shaders/AtmosphericScattering/skyfromatmosphere.frag"});
        /*
        SunDepthTest = new glshaderprogram({"shaders/AtmosphericScattering/sundepthtest.vert","shaders/AtmosphericScattering/sundepthtest.frag"});
        BlurH = new glshaderprogram({"shaders/AtmosphericScattering/blur.vert","shaders/AtmosphericScattering/blurh.frag"});
        BlurV = new glshaderprogram({"shaders/AtmosphericScattering/blur.vert","shaders/AtmosphericScattering/blurv.frag"});
        SunRaysLenseFlareHalo = new glshaderprogram({"shaders/AtmosphericScattering/blur.vert","shaders/AtmosphericScattering/sunrayslensflare.frag"});
    
        // Set default Uniforms
        SunDepthTest->use();
        glUniform1i(SunDepthTest->getUniformLocation("SunTexture"),0);
        glUniform1i(SunDepthTest->getUniformLocation("DepthTexture"),1);
        glUseProgram(0);

        SunRaysLenseFlareHalo->use();
        glUniform1i(SunRaysLenseFlareHalo->getUniformLocation("LowBlurredSunTexture"),0);
        glUniform1i(SunRaysLenseFlareHalo->getUniformLocation("HighBlurredSunTexture"),1);
        glUniform1i(SunRaysLenseFlareHalo->getUniformLocation("DirtTexture"),2);
        glUniform1f(SunRaysLenseFlareHalo->getUniformLocation("Dispersal"),0.1875f);
        glUniform1f(SunRaysLenseFlareHalo->getUniformLocation("HaloWidth"),0.45f);
        glUniform1f(SunRaysLenseFlareHalo->getUniformLocation("Intensity"),2.25f);
        glUniform3f(SunRaysLenseFlareHalo->getUniformLocation("Distortion"),0.94f,0.97f,1.00f);
        glUseProgram(0);
*/
        sky->use();
        glUniform3f(sky->getUniformLocation("v3CameraPos"),0.0f,InnerRadius,0.0f);
        glUniform3f(sky->getUniformLocation("v3InvWavelength"),1.0f / powf(0.650f, 4.0f), 1.0f / powf(0.570f, 4.0f), 1.0f / powf(0.475f, 4.0f));
        glUniform1f(sky->getUniformLocation("fCameraHeight"), InnerRadius);
        //glUniform1f(sky->getUniformLocation("fCameraHeight2"), InnerRadius * InnerRadius);
        glUniform1f(sky->getUniformLocation("fInnerRadius"), InnerRadius);
        //glUniform1f(sky->getUniformLocation("fInnerRadius2"), InnerRadius * InnerRadius);
        //glUniform1f(sky->getUniformLocation("fOuterRadius"), OuterRadius);
        //glUniform1f(sky->getUniformLocation("fOuterRadius2"), OuterRadius * OuterRadius);
        glUniform1f(sky->getUniformLocation("fKrESun"), Kr * ESun);
        glUniform1f(sky->getUniformLocation("fKmESun"), Km * ESun);
        glUniform1f(sky->getUniformLocation("fKr4PI"), Kr * 4.0f * (float)M_PI);
        glUniform1f(sky->getUniformLocation("fKm4PI"), Km * 4.0f * (float)M_PI);
        glUniform1f(sky->getUniformLocation("fScale"), Scale);
        glUniform1f(sky->getUniformLocation("fScaleDepth"), ScaleDepth);
        glUniform1f(sky->getUniformLocation("fScaleOverScaleDepth"), ScaleOverScaleDepth);
        glUniform1f(sky->getUniformLocation("g"), g);
        glUniform1f(sky->getUniformLocation("g2"), g * g);
        glUniform1i(sky->getUniformLocation("Samples"), 4);        
        glUseProgram(0);
    
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void initAS(){
    try {     
        
        vec3* SkyDomeVertices = new vec3[112 * 3],va,vb,vc,vd;
        float stepa = (float)M_PI * 2.0f / 16, startb = asin(InnerRadius/OuterRadius), stepb = ((float)M_PI_2 - startb) / 4;
        int pos = 0;
        
        for(int y = 0; y < 3; y++){
            float b = startb + stepb * y;
            for(int x = 0; x < 16; x++){
                
                float a = stepa * x;                
                va = vec3(sin(a) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a) * cos(b) * OuterRadius);
                vb = vec3(sin(a + stepa) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a + stepa) * cos(b) * OuterRadius);
                vc = vec3(sin(a + stepa) * cos(b + stepb) * OuterRadius, sin(b + stepb) * OuterRadius, -cos(a + stepa) * cos(b + stepb) * OuterRadius);
                vd = vec3(sin(a) * cos(b + stepb) * OuterRadius, sin(b + stepb) * OuterRadius, -cos(a) * cos(b + stepb) * OuterRadius);

                SkyDomeVertices[pos + 0] = va;
                SkyDomeVertices[pos + 1] = vb;
                SkyDomeVertices[pos + 2] = vc;

                pos += 3;

                SkyDomeVertices[pos + 0] = vc;
                SkyDomeVertices[pos + 1] = vd;
                SkyDomeVertices[pos + 2] = va;

                pos += 3;
            }
        }

        float b = startb + stepb * 3;

        for(int x = 0; x < 16; x++){
            float a = stepa * x;
            
            va = vec3(sin(a) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a) * cos(b) * OuterRadius);
		    vb = vec3(sin(a + stepa) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a + stepa) * cos(b) * OuterRadius);
		    vc = vec3(0, OuterRadius, 0);

		    SkyDomeVertices[pos + 0] = va;
		    SkyDomeVertices[pos + 1] = vb;
		    SkyDomeVertices[pos + 2] = vc;
    		pos += 3;
        }

        glCreateVertexArrays(1,&vao);
        glBindVertexArray(vao);
        glGenBuffers(1,&vbo);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,112 * 3 * 3 * sizeof(float), SkyDomeVertices,GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);

        delete [] SkyDomeVertices;

        dirtTexture = createTexture2D("resources/textures/lensdirt_lowc.jpg");

        glGenFramebuffers(1,&fbo);
        glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        glGenTextures(1, &DepthTexture);
        glGenTextures(4, SunTextures);
        glGenRenderbuffers(1, &rbo);
	    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512);
	    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
	    const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	    glDrawBuffers(3, buffers);
	    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		    throwErr("HDR Framebuffer Not Complete !!!");
	    }
    	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void renderAS(camera *cam,vec3 camPos){
    try {

        vec3 SunCDir = normalize(SunCPos);
        vec3 SunWPos = camPos + SunCPos;
        float RefractionFactor = (1.0f - sqrt(max(0.0f,SunCDir[1])));
        vec3 SunColor = vec3(1.0f) - vec3(0.0,0.5,1.0f) * RefractionFactor;

        sky->use();
        glBindVertexArray(vao);
        glUniform3fv(sky->getUniformLocation("v3LightPos"),1,SunCDir);
        glUniformMatrix4fv(sky->getUniformLocation("mvpMatrix"),1,GL_FALSE,programglobal::perspective * cam->matrix() * translate(0.0f,0.0f,0.0f));
        glDrawArrays(GL_TRIANGLES,0,112*3);
        glUseProgram(0);
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void uninitAS(){

    if(sky)
        delete sky;
    if(SunDepthTest)
        delete SunDepthTest;
    if(BlurH)
        delete BlurH;
    if(BlurV)
        delete BlurV;
    if(SunRaysLenseFlareHalo)
        delete SunRaysLenseFlareHalo;
}
