#include <atmosphere.h>
#include <cmath>

Atmosphere::Atmosphere(glmodel* tsphereModel)
:sphereModel(tsphereModel)
{
    this->SetDefaults();
    atmosphereProgram = new glshaderprogram({"shaders/AtmosphericScattering/Atmosphere.vert","shaders/AtmosphericScattering/Atmosphere.frag"});
}

void Atmosphere::SetDefaults(){

    viewSamples = defViewSamples;
    lightSamples = defLightSamples;

    sunDir = defSunDir;
    setSunAngle(defSunAngle);
    I_Sun = e_I_sun;
    setEarthRadius(e_R_e);
    setAtmosRadius(e_R_a);
    beta_R = e_beta_R;
    beta_M = e_beta_M;

    H_R = e_H_R;
    H_M = e_H_M;
    g = e_g;
}

void Atmosphere::SetSunDefaults(){

    animateSun = false;
    sunDir = defSunDir;
    setSunAngle(defSunAngle);
    I_Sun = e_I_sun;
}

void Atmosphere::SetRayLightDefaults(){
    beta_R = e_beta_R;
    H_R = e_H_R;
}

void Atmosphere::SetMieDefaults(){
    beta_M = e_beta_M;
    H_M = e_H_M;
    g = e_g;
}

void Atmosphere::SetsizeDefaults(){
    setEarthRadius(e_R_e);
    setAtmosRadius(e_R_a);
}

void Atmosphere::render(float dt){

    try{



        // 1 Set Properties of the atmosphere
        atmosphereProgram->use();
        glUniformMatrix4fv(atmosphereProgram->getUniformLocation("M"),1,GL_FALSE,modelAtmos);
        glUniformMatrix4fv(atmosphereProgram->getUniformLocation("MVP"),1,GL_FALSE,proj * view * modelAtmos);

        glUniform3fv(atmosphereProgram->getUniformLocation("viewPos"),1,viewPos);
        glUniform1i(atmosphereProgram->getUniformLocation("viewSamples"),viewSamples);
        glUniform1i(atmosphereProgram->getUniformLocation("lightSamples"),lightSamples);

        glUniform1f(atmosphereProgram->getUniformLocation("I_sun"),I_Sun);
        glUniform1f(atmosphereProgram->getUniformLocation("R_e"),R_e);
        glUniform1f(atmosphereProgram->getUniformLocation("R_a"),R_a);
        glUniform3fv(atmosphereProgram->getUniformLocation("beta_R"),1,beta_R);
        glUniform1f(atmosphereProgram->getUniformLocation("beta_M"),beta_M);
        glUniform1f(atmosphereProgram->getUniformLocation("H_R"),H_R);
        glUniform1f(atmosphereProgram->getUniformLocation("H_M"),H_M);
        glUniform1f(atmosphereProgram->getUniformLocation("g"),g);

        if(animateSun){
            double pp = M_PI + vmath::radians(20.0f);
            sunAngle = modf(sunAngle + 0.5 * dt,&pp);
            sunDir[1] = sinf(sunAngle);
            sunDir[2] = cosf(sunAngle);
        }

        glUniform3fv(atmosphereProgram->getUniformLocation("sunPos"),1,sunDir);
        sphereModel->draw(atmosphereProgram,1,false);

    }
    catch(std::string errorString){
        throwErr(errorString);
    }

}
