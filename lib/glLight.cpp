#include<cmath>
#include<cstddef>
#include<glshaderloader.h>
#include<glLight.h>
#include<iostream>
#include<string>
#include<errorlog.h>
#include<X11/keysym.h>
#include<CubeMapRenderTarget.h>
#include<global.h>

using namespace std;
using namespace vmath;

static int mode = 0;
static int selectedLight = 0;

SceneLight::SceneLight(bool envLight) :
	envProgram(nullptr),
	irradianceProgram(nullptr),
	prefilterProgram(nullptr),
	precomputeBRDF(nullptr),
	irradianceMap(nullptr),
	prefilterMap(nullptr) {
	const float cubeVerts[] = {
		-0.5f, -0.5f, -0.5f, 
		0.5f, -0.5f, -0.5f, 
		0.5f,  0.5f, -0.5f, 
		0.5f,  0.5f, -0.5f, 
		-0.5f,  0.5f, -0.5f, 
		-0.5f, -0.5f, -0.5f, 
		-0.5f, -0.5f, 0.5f,  
		0.5f, -0.5f, 0.5f,  
		0.5f,  0.5f, 0.5f,  
		0.5f,  0.5f, 0.5f, 
		-0.5f,  0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
												
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
												
		0.5f,  0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f,  0.5f,  0.5f,
												
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,
		0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,
											
		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f,  0.5f,
		0.5f, 0.5f,  0.5f,
		-0.5f, 0.5f,  0.5f,
		-0.5f, 0.5f, -0.5f
	};
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	GLuint vbo;
	glCreateBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	//glDeleteBuffers(1,&vbo);

		// Skybox 
		float skybox_positions[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	
	glCreateVertexArrays(1, &this->skybox);
	glBindVertexArray(this->skybox);
	glCreateBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_positions), skybox_positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	//setup light program
	this->indirectLight = envLight;
	if(envLight){
		try
		{
			envProgram = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/debug/rendercubemap.frag"});
			irradianceProgram = new glshaderprogram({"shaders/debug/rendercubemap.vert","shaders/debug/irradiance_convolution.frag"});
			prefilterProgram = new glshaderprogram({"shaders/debug/rendercubemap.vert","shaders/debug/prefilter_cubemap.frag"});
			precomputeBRDF = new glshaderprogram({"shaders/fsquad.vert","shaders/debug/precompute_brdf.frag"});
			lightRender = new glshaderprogram({"shaders/debug/lightSrc.vert", "shaders/debug/lightSrc.frag"});

			//envirounmentMap = new CubeMapRenderTarget(2048,2048,false);
			irradianceMap = new CubeMapRenderTarget(32,32,false);
			prefilterMap = new CubeMapRenderTarget(512,512,true);

			irradianceMap->setPosition(vec3(0.0));
			prefilterMap->setPosition(vec3(0.0));

			this->brdf.width = 512;
			this->brdf.height = 512;

			// Setup Precompute BRDF Texture
			glGenTextures(1, &this->brdf.brdfTex);
			glBindTexture(GL_TEXTURE_2D,this->brdf.brdfTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, this->brdf.width, this->brdf.height, 0, GL_RG, GL_FLOAT, 0);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glGenFramebuffers(1,&this->brdf.fbo);
			glBindFramebuffer(GL_FRAMEBUFFER,this->brdf.fbo);
			glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,this->brdf.brdfTex,0);

			glGenRenderbuffers(1,&this->brdf.rbo);
			glBindRenderbuffer(GL_RENDERBUFFER,this->brdf.rbo);
			glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,this->brdf.width,this->brdf.height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,this->brdf.rbo);
			glBindTexture(GL_TEXTURE_2D,0);
			glBindRenderbuffer(GL_RENDERBUFFER,0);
			glBindFramebuffer(GL_FRAMEBUFFER,0);            
		}
		catch(string errorString){
			throwErr(errorString);
		}
	}
	ambient = vec3(0.0f);
}

SceneLight::~SceneLight(){
	glDeleteVertexArrays(1,&vao);
	
	if(envProgram)
		delete envProgram;

	if(irradianceProgram)
		delete irradianceProgram;
	
	if(prefilterProgram)
		delete prefilterProgram;

	if(precomputeBRDF)
		delete precomputeBRDF;

	if(irradianceMap)
		delete irradianceMap;

	if(prefilterMap)
		delete prefilterMap;
}

void SceneLight::setEnvmap(GLuint &tex){
	//this->envirounmentMap = envMap;
	this->envirounmentMap = tex;
	// set irradiance and prefilter positions
	//this->irradianceMap->setPosition(this->envirounmentMap->getPosition());
	//this->prefilterMap->setPosition(this->envirounmentMap->getPosition());
}

void SceneLight::PrecomputeIndirectLighting(){

	// Irradiance Convolution
	glBindFramebuffer(GL_FRAMEBUFFER,irradianceMap->FBO);
	glViewport(0, 0, irradianceMap->width, irradianceMap->height);
	for(int side = 0; side < 6; side++){
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, irradianceMap->cubemap_texture,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		irradianceProgram->use();
		glUniformMatrix4fv(irradianceProgram->getUniformLocation("pMat"),1,GL_FALSE,irradianceMap->projection);
		glUniformMatrix4fv(irradianceProgram->getUniformLocation("vMat"),1,GL_FALSE,irradianceMap->view[side]);
		glBindTextureUnit(0,this->envirounmentMap);
		glBindVertexArray(this->skybox);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	//Prefilter map
	glBindFramebuffer(GL_FRAMEBUFFER,prefilterMap->FBO);
	GLuint max_mip_level = log2(float(prefilterMap->width));
	for(GLuint i = 0; i < max_mip_level; i++){
		
		GLuint mip_width = prefilterMap->width * pow(0.5,i);
		GLuint mip_height = prefilterMap->height * pow(0.5, i);

		glBindRenderbuffer(GL_RENDERBUFFER,prefilterMap->RBO);
		glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,mip_width,mip_height);
		glViewport(0, 0, mip_width, mip_height);

		float roughness = float(i) / float(max_mip_level - 1);
		for(int side = 0; side < 6; side++){
			
			glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + side, prefilterMap->cubemap_texture,i);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			prefilterProgram->use();
			glUniformMatrix4fv(prefilterProgram->getUniformLocation("pMat"),1,GL_FALSE,prefilterMap->projection);
			glUniformMatrix4fv(prefilterProgram->getUniformLocation("vMat"),1,GL_FALSE,prefilterMap->view[side]);
			glUniform1f(prefilterProgram->getUniformLocation("u_roughness"),roughness);
			glBindTextureUnit(0,this->envirounmentMap);
			glBindVertexArray(this->skybox);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	GLuint temp;
	glCreateVertexArrays(1,&temp);
	glBindFramebuffer(GL_FRAMEBUFFER,brdf.fbo);
	glViewport(0, 0, 512, 512);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	precomputeBRDF->use();
	glBindVertexArray(temp);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glDeleteVertexArrays(1,&temp);
}

void SceneLight::setAmbient(vec3 ambient) {
	this->ambient = ambient;
}

void SceneLight::addDirectionalLights(vector<DirectionalLight> dl){
	this->directional.insert(this->directional.end(), dl.begin(), dl.end());
}

void SceneLight::addPointLights(vector<PointLight> pl){
	this->points.insert(this->points.end(), pl.begin(), pl.end());
}

void SceneLight::addSpotLights(vector<SpotLight> sl){
	this->spots.insert(this->spots.end(), sl.begin(), sl.end());
}

void SceneLight::setLightUniform(glshaderprogram *program, bool useIndirectLight){
	// Directional Lights
	glUniform1i(program->getUniformLocation("numOfDL"),directional.size());
	for(size_t i = 0; i < directional.size(); i++){
		glUniform3fv(program->getUniformLocation("dl["+to_string(i)+"].base.color"),1,directional[i].color);
		glUniform3fv(program->getUniformLocation("dl["+to_string(i)+"].direction"),1, normalize(directional[i].direction));
		glUniform1f(program->getUniformLocation("dl["+to_string(i)+"].base.intensity"),directional[i].intensity);
	}

	glUniform1i(program->getUniformLocation("numOfPoints"),points.size());
	for(size_t i = 0; i < points.size(); i++){
		
		glUniform3fv(program->getUniformLocation("pl["+to_string(i)+"].base.color"),1,points[i].color);
		glUniform3fv(program->getUniformLocation("pl["+to_string(i)+"].position"),1,points[i].position);
		glUniform1f(program->getUniformLocation("pl["+to_string(i)+"].base.intensity"),points[i].intensity);
		glUniform1f(program->getUniformLocation("pl["+to_string(i)+"].radius"),points[i].radius);
	}

	glUniform1i(program->getUniformLocation("numOfSpots"),spots.size());
	for(size_t i = 0; i < spots.size(); i++){
		
		glUniform3fv(program->getUniformLocation("sl["+to_string(i)+"].point.base.color"),1,spots[i].color);
		glUniform3fv(program->getUniformLocation("sl["+to_string(i)+"].point.position"),1,spots[i].position);
		glUniform3fv(program->getUniformLocation("sl["+to_string(i)+"].direction"),1,spots[i].direction);
		glUniform1f(program->getUniformLocation("sl["+to_string(i)+"].point.base.intensity"),spots[i].intensity);
		glUniform1f(program->getUniformLocation("sl["+to_string(i)+"].point.radius"),spots[i].radius);
		glUniform1f(program->getUniformLocation("sl["+to_string(i)+"].inner_angle"),spots[i].inner_angle);
		glUniform1f(program->getUniformLocation("sl["+to_string(i)+"].outer_angle"),spots[i].outer_angle);
	}

	if(this->indirectLight && useIndirectLight){   
		glUniform1i(program->getUniformLocation("IBL"),this->indirectLight);
		glBindTextureUnit(8,irradianceMap->cubemap_texture);
		glBindTextureUnit(9,prefilterMap->cubemap_texture);
		glBindTextureUnit(10,brdf.brdfTex);
	} else {
		glUniform3fv(program->getUniformLocation("ambientColor"), 1, ambient);
	}
}

void SceneLight::renderSceneLights() {
	this->lightRender->use();
	glUniformMatrix4fv(this->lightRender->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
	glUniformMatrix4fv(this->lightRender->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix()); 
		
	// directional lights
	for(size_t i = 0; i < directional.size(); i++){
		glUniformMatrix4fv(this->lightRender->getUniformLocation("mMat"),1,GL_FALSE,translate(directional[i].direction) * scale(0.1f,0.1f,0.1f));
		glUniform3fv(this->lightRender->getUniformLocation("color"),1,directional[i].color);
		if(mode == 0 && i == selectedLight)
			glUniform1i(this->lightRender->getUniformLocation("selected"),GL_TRUE);
		else
			glUniform1i(this->lightRender->getUniformLocation("selected"),GL_FALSE);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	// render point lights
	for(size_t p = 0; p < points.size(); p++){
		glUniformMatrix4fv(this->lightRender->getUniformLocation("mMat"),1,GL_FALSE,translate(points[p].position) * scale(points[p].radius/100.0f));
		glUniform3fv(this->lightRender->getUniformLocation("color"),1,points[p].color);
		if(mode == 1 && p == selectedLight)
			glUniform1i(this->lightRender->getUniformLocation("selected"),GL_TRUE);
		else
			glUniform1i(this->lightRender->getUniformLocation("selected"),GL_FALSE);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	// render spot lights
	for(size_t s = 0; s < spots.size(); s++){
		glUniformMatrix4fv(this->lightRender->getUniformLocation("mMat"),1,GL_FALSE,translate(spots[s].position) * scale(spots[s].radius/100.0f));
		glUniform3fv(this->lightRender->getUniformLocation("color"),1,spots[s].color);
		if(mode == 2 && s == selectedLight)
			glUniform1i(this->lightRender->getUniformLocation("selected"),GL_TRUE);
		else
			glUniform1i(this->lightRender->getUniformLocation("selected"),GL_FALSE);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void SceneLight::SceneLightKeyBoardFunc(int key){
	switch(key) {
		case XK_b: // add Directional Light
			mode = 0;
		break;
		case XK_n: // Add Point Light
			mode = 1;
		break;
		case XK_m: // Add Spot Light
			mode = 2;        
		break;
		// case XK_Up:
		//     if(mode == 0)
		//         this->addDirectionalLight(DirectionalLight(vec3(0.1f),10.0f,vec3(0.0,0.0,0.0f)));
		//     else if(mode == 1)
		//         this->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(0.0f,0.0f,0.0f),10.0f));
		//     else
		//         this->addSpotLight(SpotLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(0.0f,0.0f,0.0f),10.0f,vec3(0.0f,0.0f,0.0f),30.0f,45.0f));
		// break;
		// case XK_Down:
		//     if(mode == 0)
		//         this->directional.erase(directional.begin() + selectedLight);
		//     else if(mode == 1)
		//         this->points.erase(points.begin() + selectedLight);
		//     else
		//         this->spots.erase(spots.begin() + selectedLight);
		// break;
		case XK_Left:
			if(mode == 0 && directional.size() > 0){
				selectedLight = (selectedLight + 1) % directional.size();
			}
			else if(mode == 1 && points.size() > 0){
				selectedLight = (selectedLight + 1) % points.size();
			}
			else if(mode == 2 && spots.size() > 0) {
				selectedLight = (selectedLight + 1) % spots.size();
			}
		break;
		case XK_Right:
			if(mode == 0 && directional.size() > 0) {
				selectedLight = (selectedLight == 0) ? directional.size() - 1 : (selectedLight - 1);
			} else if(mode == 1 && points.size() > 0) {
				selectedLight = (selectedLight == 0) ? points.size() - 1 : (selectedLight - 1);
			} else if(mode == 2 && spots.size()) {
				selectedLight = (selectedLight == 0) ? spots.size() - 1 : (selectedLight - 1);
			}
		break;
		case XK_I:
		case XK_i:
			if(mode == 0 && directional.size() > 0) {
				directional[selectedLight].direction[2] -= 0.1f;
			} else if(mode == 1 && points.size() > 0) {
				points[selectedLight].position[2] -= 0.1f; 
			} else if(mode == 2 && spots.size()) {
				spots[selectedLight].position[2] -= 0.1f;
			}
		break;
		case XK_J:
		case XK_j:
			if(mode == 0 && directional.size() > 0) {
				directional[selectedLight].direction[0] -= 0.1f;
			} else if(mode == 1 && points.size() > 0) {
				points[selectedLight].position[0] -= 0.1f; 
			} else if(mode == 2 && spots.size()) {
				spots[selectedLight].position[0] -= 0.1f;
			}
			break;
		case XK_K:
		case XK_k:
			if(mode == 0 && directional.size() > 0) {
				directional[selectedLight].direction[2] += 0.1f;
			} else if(mode == 1 && points.size() > 0) {
				points[selectedLight].position[2] += 0.1f; 
			} else if(mode == 2 && spots.size()) {
				spots[selectedLight].position[2] += 0.1f;
			}
		break;
		case XK_L:
		case XK_l:
			if(mode == 0 && directional.size() > 0) {
				directional[selectedLight].direction[0] += 0.1f;
			} else if(mode == 1 && points.size() > 0) {
				points[selectedLight].position[0] += 0.1f; 
			} else if(mode == 2 && spots.size()) {
				spots[selectedLight].position[0] += 0.1f;
			}
		break;
		case XK_U:
		case XK_u:
			if(mode == 0 && directional.size() > 0) {
				directional[selectedLight].direction[1] += 0.1f;
			} else if(mode == 1 && points.size() > 0) {
				points[selectedLight].position[1] += 0.1f; 
			} else if(mode == 2 && spots.size()) {
				spots[selectedLight].position[1] += 0.1f;
			}
		break;
		case XK_O:
		case XK_o:
			if(mode == 0 && directional.size() > 0) {
				directional[selectedLight].direction[1] -= 0.1f;
			} else if(mode == 1 && points.size() > 0) {
				points[selectedLight].position[1] -= 0.1f; 
			} else if(mode == 2 && spots.size()) {
				spots[selectedLight].position[1] -= 0.1f;
			}
		break;
		case XK_bracketleft:
			if(mode == 1 && points.size() > 0) {
				points[selectedLight].radius -= 0.5f;
			}
		break;
		case XK_bracketright:
			if(mode == 1 && points.size() > 0) {
				points[selectedLight].radius += 0.5f;
			}
		break;
		case XK_comma:
			if(mode == 0 && directional.size() > 0) {
				directional[selectedLight].intensity -= 1.0f;
			} else if(mode == 1 && points.size() > 0) {
				points[selectedLight].intensity -= 1.0f; 
			} else if(mode == 2 && spots.size()) {
				spots[selectedLight].intensity -= 1.0f;
			}
		break;
		case XK_period:
			if(mode == 0 && directional.size() > 0) {
				directional[selectedLight].intensity += 1.0f;
			} else if(mode == 1 && points.size() > 0) {
				points[selectedLight].intensity += 1.0f; 
			} else if(mode == 2 && spots.size()) {
				spots[selectedLight].intensity += 1.0f;
			}
		break;
	}
}

ostream& operator<<(ostream& out, SceneLight* s) {
	
	out<<"Scene Light"<<"\n";
	out<<"Directional "<<s->directional.size()<<"\n";

	for(auto d : s->directional)
	{
		out<<"DirectionalLight(vec3("<<d.color<<"),"<<d.intensity<<".0f,vec3("<<d.direction<<"))\n";
	}

	out<<"\n";
	out<<"Point "<<s->points.size()<<"\n";
	for(auto p : s->points)
	{
		out<<"PointLight(vec3("<<p.color<<"),"<<p.intensity<<"f,vec3("<<p.position<<"),"<<p.radius<<"f)\n";
	}
	out<<"\n";
	out<<"Spot "<<s->spots.size()<<"\n";
	for(auto s : s->spots)
	{
		out<<"SpotLight(vec3("<<s.color<<"),"<<s.intensity<<"f,vec3("<<s.position<<"),"<<s.radius<<"f,vec3("
		<<s.direction<<",)"<<s.inner_angle<<".0f,"<<s.outer_angle<<".0f)\n";
	}
/*    out<<"To load: "<<s->translateBy<<", "<<modelplacer->rotateBy<<", "<<modelplacer->scaleBy<<"f\n";
	
	out<<"To use : translate("<<modelplacer->translateBy[0]<<"f, "<<modelplacer->translateBy[1]<<"f, "<<modelplacer->translateBy[2]<<"f) * ";
	if(modelplacer->rotateBy[0] != 0.0f) {
		out<<"rotate("<<modelplacer->rotateBy[0]<<"f, 1.0f, 0.0f, 0.0f) * ";
	}
	if(modelplacer->rotateBy[1] != 0.0f) {
		out<<"rotate("<<modelplacer->rotateBy[1]<<"f, 0.0f, 1.0f, 0.0f) * ";
	}
	if(modelplacer->rotateBy[2] != 0.0f) {
		out<<"rotate("<<modelplacer->rotateBy[2]<<"f, 0.0f, 0.0f, 1.0f) * ";
	}
	out<<"scale("<<modelplacer->scaleBy<<"f);";
*/
	return out;
}

void SceneLight::setDirectionalLightColor(int i, vmath::vec3 color) {
	this->directional[i].update_color(color);
}

void SceneLight::setDirectionalLightDirection(int i, vmath::vec3 direction) {
	this->directional[i].setDirection(direction);
}