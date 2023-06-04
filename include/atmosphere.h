#ifndef __ATMOSPPHERE__
#define __ATMOSPPHERE__

#include<glmodelloader.h>
#include<string>
#include<vmath.h>
#include<debugcamera.h>
#include<CubeMapRenderTarget.h>
#include<glshaderloader.h>
#include<errorlog.h>


class Atmosphere {
    private:
        
        glshaderprogram* atmosphereProgram;
        glmodel* sphereModel;

        debugCamera *debugcamera;

        vmath::mat4 modelAtmos;
        vmath::mat4 modelEarth;

        vmath::mat4 proj;
        vmath::mat4 view;

        vmath::vec3 viewPos;
        int viewSamples;
        int lightSamples;

        bool animateSun = false;
        float sunAngle;

        // Atmospheric Constatns
        vmath::vec3 sunDir; // Direction of light ray
        float I_Sun;    // Intensity of the sun
        float R_e;  // Radius of the earth
        float R_a;  // Radius of the atmospphere

        vmath::vec3 beta_R; // Raylight scattering coefficient
        float H_R;  // Raylight Scale Height

        float beta_M; // Mie Scattering Coefficient
        float H_M;  // mie scattering coefficient
        float g; // Mie Scattering direction - anisotropy of the medium

        // Defaults

        inline static const int defViewSamples = 16;
        inline static const int defLightSamples = 8;
        inline static const int defSunAngle = vmath::radians(1.0f);

        inline static const vmath::vec3 defSunDir = vmath::vec3(0.0f,0.017f,-1.0f);

        // Earth presets in [km]
        inline static const float e_I_sun = 20.f;
        inline static const float e_R_e = 6360.00;       // 6360e3
        inline static const float e_R_a = 6420.00;       // 6420e3;
        inline static const vmath::vec3 e_beta_R = vmath::vec3(0.0058f, 0.0135f, 0.0331f);
        //(3.8e-6f, 13.5e-6f, 33.1e-6f);    // scrathapixel implementation
        //(5.8e-6f, 13.5e-6f, 33.1e-6f);    // scrathapixel web
        inline static const float e_beta_M = 0.0210f;          // 21e-6f
        inline static const float e_H_R = 7.994;            // 7994, 100
        inline static const float e_H_M = 1.200;            // 1200, 20
        inline static const float e_g = 0.888;

        // Conversions
        inline static const float M_2_KM = 0.001;
        inline static const float KM_2_M = 1000.0; 
    public:

        Atmosphere();
        ~Atmosphere();
        void SetDefaults();
        void SetSunDefaults();
        void SetRayLightDefaults();
        void SetMieDefaults();
        void SetsizeDefaults();

        void render(float delta);

        // Getters

        const vmath::vec3& getViewPos(){return viewPos;}
        const vmath::mat4& getModelEarth() {return modelEarth;}
        int getViewSamples() {return viewSamples;}
        int getLightSamples() {return lightSamples;}

        bool isAnimateSun() {return animateSun;}
        float getSunAngle(){return sunAngle;}

        float getEarthRadius() { return R_e; }
        float getAtmosRadius() { return R_a; }

        const vmath::vec3& getSunDir() {return sunDir;}
        float getRayLightScaleHeight(){return H_R;}

        float getMieScaattering() {return beta_M;}
        float getMieScaleHeight() { return H_M; }
        float getMieScatteringDir() { return g; }

        // Setters

        void setProjView(const vmath::mat4& tproj, const vmath::mat4& tview){
            proj = tproj;
            view = tview;
        }

        void setViewPos(const vmath::vec3& camPos){
            viewPos = camPos;
        }

        void setViewSamples(int samples){viewSamples = samples;}
        void setLightSamples(int samples){lightSamples = samples;}

        void setAnimateSun(bool b){animateSun = b;}
        void setSunAngle(float angle){
            sunAngle = angle;
            sunDir[1] = sinf(sunAngle);
            sunDir[2] = -cosf(sunAngle);
        }

        void setSunDir(const vmath::vec3& dir){sunDir = dir;}
        void setSunIntensity(float I){I_Sun = I;}

        void setEarthRadius(float R){
            R_e = R;
            modelEarth = vmath::scale(R_e,R_e,R_e);
        }

        void setAtmosRadius(float R){
            R_a = R;
            modelAtmos = vmath::scale(R_a,R_a,R_a);
        }

        void setRayLightScattering(const vmath::vec3 beta_s){beta_R = beta_s;}
        void setRayLightScaleHeight(float H){H_R = H;}

        void setMieScattering(float beta_s){beta_M = beta_s;}
        void setMieScaleHeight(float H){H_M = H;}
        void setMieScatteringDir(float d){g = d;}
};

#endif // __ATMOSPPHERE__

