#include <X11/keysym.h>

#include<glshaderloader.h>
#include<gltextureloader.h>
#include<vmath.h>
#include<errorlog.h>
#include<hdr.h>

using namespace std;
using namespace vmath;
static GLuint tempVao;

HDR::HDR(GLfloat exposure, GLfloat fade, GLsizei size) : threshhold(1.5f) ,knee(0.1f) ,bloom_intensity(1.0f), bloomEnabled(false) {
	this->exposure = exposure;
	this->fade = fade;
	this->size = size;
	// calculate max mip map levels
	GLuint width = size/2;
	GLuint height = size / 2;
	this->mipLevels = 1;

	for(size_t i = 0; i < 16; i++){
		width = width / 2;
		height = height / 2;

		if(width < 10 || height < 10)
			break;
		++this->mipLevels;
	}
}

void HDR::setupProgram(void) {
	try {
		this->hdrprogram = new glshaderprogram({"shaders/fsquad.vert", "shaders/hdr.frag"});
		this->downscaleprogram = new glshaderprogram({"shaders/downscale.comp"});
		this->upscaleprogram = new glshaderprogram({"shaders/upscale.comp"}); 
	} catch (string errorString) {
		throwErr(errorString);
	}
}

void HDR::init(void) {

	glGenVertexArrays(1, &tempVao);
	glGenFramebuffers(1, &this->FBO);
	
	glGenTextures(1, &this->colorTex);
	glBindTexture(GL_TEXTURE_2D, this->colorTex);
	glTexStorage2D(GL_TEXTURE_2D, this->mipLevels, GL_RGBA32F, this->getSize(),this->getSize());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	glGenTextures(1, &this->emisionTex);
	glBindTexture(GL_TEXTURE_2D, this->emisionTex);
	glTexStorage2D(GL_TEXTURE_2D, this->mipLevels, GL_RGBA32F, this->getSize(),this->getSize());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	glGenTextures(1, &this->occlusionTex);
	glBindTexture(GL_TEXTURE_2D, this->occlusionTex);
	glTexStorage2D(GL_TEXTURE_2D, this->mipLevels, GL_RGBA8, this->getSize(),this->getSize());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	glGenRenderbuffers(1, &this->RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->getSize(), this->getSize());

	glBindFramebuffer(GL_FRAMEBUFFER, this->FBO);
	glBindTexture(GL_TEXTURE_2D, this->colorTex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->colorTex, 0);

	glBindTexture(GL_TEXTURE_2D, this->emisionTex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->emisionTex, 0);
	
	glBindTexture(GL_TEXTURE_2D, this->occlusionTex);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, this->occlusionTex, 0);
	
	glBindRenderbuffer(GL_RENDERBUFFER, this->RBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->RBO);
	const GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, buffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throwErr("HDR Framebuffer Not Complete !!!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HDR::render(void) {

	if(this->bloomEnabled)
	{
		// Bloom pass / Blur Pass
		// Down scale pass
		this->downscaleprogram->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->emisionTex);
		uvec2 mip_size = uvec2(this->getSize()/2,this->getSize()/2);
		for(size_t i = 0; i < this->mipLevels - 1; ++i){
			
			glUniform2fv(this->downscaleprogram->getUniformLocation("u_texel_size"),1,1.0f/vec2(mip_size[0],mip_size[1]));
			glUniform1i(this->downscaleprogram->getUniformLocation("u_mip_level"),i);
			glBindImageTexture(0,this->emisionTex,i+1,GL_FALSE,0,GL_WRITE_ONLY,GL_RGBA32F);
			glDispatchCompute(ceil(float(mip_size[0])/8),ceil(float(mip_size[1])/8),1);
			mip_size = mip_size / 2u;
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		// Up Scale Pass
		this->upscaleprogram->use();
		glUniform1f(this->upscaleprogram->getUniformLocation("u_bloom_intensity"),this->bloom_intensity);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->emisionTex);
		for(size_t i = this->mipLevels - 1; i >= 1; --i){
			
			mip_size[0] = max(1.0f, float(this->getSize()/pow(2,i-1)));
			mip_size[1] = max(1.0f, float(this->getSize()/pow(2,i-1)));

			glUniform2fv(this->upscaleprogram->getUniformLocation("u_texel_size"),1,1.0f/vec2(mip_size[0],mip_size[1]));
			glUniform1i(this->upscaleprogram->getUniformLocation("u_mip_level"),i);
			glBindImageTexture(0,this->emisionTex,i-1,GL_FALSE,0,GL_READ_WRITE,GL_RGBA32F);
			glDispatchCompute(ceil(float(mip_size[0])/8),ceil(float(mip_size[1])/8),1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		}
	}
	this->hdrprogram->use();
	glUniform1i(this->hdrprogram->getUniformLocation("hdrTex"), 0);
	glUniform1i(this->hdrprogram->getUniformLocation("bloomTex"), 1);
	glUniform1f(this->hdrprogram->getUniformLocation("exposure"), this->exposure);
	glUniform1f(this->hdrprogram->getUniformLocation("fade"), this->fade);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->colorTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->emisionTex);
	glBindVertexArray(tempVao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void HDR::setBloom(bool value){
	this->bloomEnabled = value;
}

GLuint HDR::getFBO(void) {
	return this->FBO;
}

GLsizei HDR::getSize(void) {
	return this->size;
}

void HDR::updateExposure(GLfloat delta) {
	this->exposure += delta;
}

void HDR::updateBloomIntensity(GLfloat delta){
	this->bloom_intensity += delta;
}

GLfloat HDR::getExposure() {
	return this->exposure;
}

void HDR::keyboardfunc(int key) {
	switch(key) {
		case XK_e:
			this->updateExposure(-0.1f);
		break;
		case XK_r:
			this->updateExposure(0.1f);
		break;
		case XK_m:
			this->updateBloomIntensity(0.1f);
		break;
		case XK_n:
			this->updateBloomIntensity(-0.1f);
		break;
		// case XK_space:
		// 	this->bloomEnabled = !this->bloomEnabled;
		break;
	}
}

void HDR::uninit(void) {
	glDeleteFramebuffers(1, &this->FBO);
	glDeleteRenderbuffers(1, &this->RBO);
	glDeleteTextures(1, &this->colorTex);
	glDeleteTextures(1, &this->emisionTex);
	glDeleteVertexArrays(1, &tempVao);
	delete this->hdrprogram;
	delete this->downscaleprogram;
	delete this->upscaleprogram;
}

