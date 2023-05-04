#include <cmath>
#include <glshaderloader.h>
#include <glLight.h>
#include <string>
#include <errorlog.h>
#include <CubeMapRenderTarget.h>

SceneLight::SceneLight(bool envLight){
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
    //glDeleteBuffers(1,&vbo);

    this->indirectLight = envLight;
    if(envLight){
        try
        {
            envProgram = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/debug/rendercubemap.frag"});
            irradianceProgram = new glshaderprogram({"shaders/debug/rendercubemap.vert","shaders/debug/irradiance_convolution.frag"});
            prefilterProgram = new glshaderprogram({"shaders/debug/rendercubemap.vert","shaders/debug/prefilter_cubemap.frag"});
            precomputeBRDF = new glshaderprogram({"shaders/hdr.vert","shaders/debug/precompute_brdf.frag"});

            //envirounmentMap = new CubeMapRenderTarget(2048,2048,false);
            irradianceMap = new CubeMapRenderTarget(32,32,false);
            prefilterMap = new CubeMapRenderTarget(512,512,true);

            irradianceMap->setPosition(vmath::vec3(0.0));
            prefilterMap->setPosition(vmath::vec3(0.0));

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
        catch(std::string errorString){
            throwErr(errorString);
        }
    }
}

SceneLight::~SceneLight(){
    glDeleteVertexArrays(1,&vao);
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
        
        GLuint mip_width = prefilterMap->width * std::pow(0.5,i);
        GLuint mip_height = prefilterMap->height * std::pow(0.5, i);

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

void SceneLight::addDirectionalLight(DirectionalLight dl){
    this->directional.push_back(dl);
}

void SceneLight::addPointLight(PointLight pl){
    this->points.push_back(pl);
}

void SceneLight::addSpotLight(SpotLight sl){
    this->spots.push_back(sl);
}

void SceneLight::setLightUniform(glshaderprogram *program){

    // Directional Lights
    glUniform1i(program->getUniformLocation("numOfDL"),directional.size());
    for(size_t i = 0; i < directional.size(); i++){
        
        glUniform3fv(program->getUniformLocation("dl["+std::to_string(i)+"].base.color"),1,directional[i].color);
        glUniform3fv(program->getUniformLocation("dl["+std::to_string(i)+"].direction"),1,directional[i].direction);
        glUniform1f(program->getUniformLocation("dl["+std::to_string(i)+"].base.intensity"),directional[i].intensity);
    }

    glUniform1i(program->getUniformLocation("numOfPoints"),points.size());
    for(size_t i = 0; i < points.size(); i++){
        
        glUniform3fv(program->getUniformLocation("pl["+std::to_string(i)+"].base.color"),1,points[i].color);
        glUniform3fv(program->getUniformLocation("pl["+std::to_string(i)+"].position"),1,points[i].position);
        glUniform1f(program->getUniformLocation("pl["+std::to_string(i)+"].base.intensity"),points[i].intensity);
        glUniform1f(program->getUniformLocation("pl["+std::to_string(i)+"].radius"),points[i].radius);
    }

    glUniform1i(program->getUniformLocation("numOfSpots"),spots.size());
    for(size_t i = 0; i < spots.size(); i++){
        
        glUniform3fv(program->getUniformLocation("sl["+std::to_string(i)+"].point.base.color"),1,spots[i].color);
        glUniform3fv(program->getUniformLocation("sl["+std::to_string(i)+"].point.position"),1,spots[i].position);
        glUniform3fv(program->getUniformLocation("sl["+std::to_string(i)+"].direction"),1,spots[i].direction);
        glUniform1f(program->getUniformLocation("sl["+std::to_string(i)+"].point.base.intensity"),spots[i].intensity);
        glUniform1f(program->getUniformLocation("sl["+std::to_string(i)+"].point.radius"),spots[i].radius);
        glUniform1f(program->getUniformLocation("sl["+std::to_string(i)+"].inner_angle"),spots[i].inner_angle);
        glUniform1f(program->getUniformLocation("sl["+std::to_string(i)+"].outer_angle"),spots[i].outer_angle);
    }

    if(this->indirectLight){   
        glUniform1i(program->getUniformLocation("IBL"),this->indirectLight);
        glBindTextureUnit(8,irradianceMap->cubemap_texture);
        glBindTextureUnit(9,prefilterMap->cubemap_texture);
        glBindTextureUnit(10,brdf.brdfTex);
    }
}

void SceneLight::renderSceneLights(glshaderprogram *program){

    // render point lights
    for(auto p : points){
        glUniformMatrix4fv(program->getUniformLocation("mMat"),1,GL_FALSE,vmath::translate(p.position) * vmath::scale(1.0f,1.0f,1.0f));
        glUniform3fv(program->getUniformLocation("color"),1,p.color);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // render spot lights
    for(auto s : spots){
        glUniformMatrix4fv(program->getUniformLocation("mMat"),1,GL_FALSE,vmath::translate(s.position) * vmath::scale(1.0f,1.0f,1.0f));
        glUniform3fv(program->getUniformLocation("color"),1,s.color);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}


