#include <glLight.h>
#include <string>

SceneLight::SceneLight(){
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
}

SceneLight::~SceneLight(){
    glDeleteVertexArrays(1,&vao);
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


