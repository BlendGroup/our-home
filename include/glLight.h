
#include <glshaderloader.h>
#include <math.h>
#include <vector>
#include <CubeMapRenderTarget.h>
#include <vmath.h>

struct BaseLight{
    vmath::vec3 color;
    float intensity;
    BaseLight(){
        color = vmath::vec3(1.0f);
        intensity = 1.0f;
    }

    BaseLight(vmath::vec3 c,float i){
        color = c;
        intensity = i;
    }

    void update_color(vmath::vec3 c)
    {
        this->color = c;
    }

    void update_intensity(float i){
        this->intensity = i;
    }

};

struct DirectionalLight : BaseLight{

    vmath::vec3 direction;

    DirectionalLight(){
        direction = vmath::vec3(0.0f,0.0f,-1.0f);
    }

    DirectionalLight(vmath::vec3 color,float intensity,vmath::vec3 dir) : BaseLight(color, intensity)
    {
        this->direction = dir;
    }

    DirectionalLight(vmath::vec3 color,float intensity,float azimuth, float elevation) : BaseLight(color, intensity)
    {
        setDirection(azimuth, elevation);
    }

    void setDirection(float azimuth, float elevation)
    {
        
        float az = vmath::radians(azimuth);
        float el = vmath::radians(elevation);
        direction[0] = sinf(el) * cosf(az);
        direction[1] = cosf(el);
        direction[2] = sinf(el) * sinf(az);
        direction = vmath::normalize(-direction);
    }
};

struct PointLight : BaseLight{
    vmath::vec3 position;
    float radius;

    PointLight(){
        position = vmath::vec3(0.0f);
        radius = 10.0f;
    }

    PointLight(vmath::vec3 color,float intensity, vmath::vec3 position, float radius) : BaseLight(color, intensity){
        this->position = position;
        this->radius = radius;
    }

    void update_position(vmath::vec3 pos){
        this->position = pos;
    }

    void update_radius(float r){
        this->radius = r;
    }
};

struct SpotLight : PointLight{

    vmath::vec3 direction;
    float inner_angle; // also used as cutoff angle
    float outer_angle;

    SpotLight(){
        direction = vmath::vec3(0.0f,0.0f,-1.0f);
        inner_angle = 35.0f;
        outer_angle = 45.0f;
    }

    SpotLight(vmath::vec3 c, float i,vmath::vec3 p, float r,vmath::vec3 dir,float inner_angle = 35.0f, float outer_angle = 60.0f) : PointLight(c, i, p, r){

        this->direction = dir;
        this->inner_angle = inner_angle;
        this->outer_angle = outer_angle;
    }

    void setDirection(float azimuth, float elevation){
        
        float az = vmath::radians(azimuth);
        float el = vmath::radians(elevation);
        direction[0] = sinf(el) * cosf(az);
        direction[1] = cosf(el);
        direction[2] = sinf(el) * sinf(az);
        direction = vmath::normalize(-direction);
    }
};

class SceneLight{

    private:

    struct brdf_lut{
        GLuint brdfTex;
        GLuint fbo,rbo;
        GLuint width,height;
    };
    GLuint vao,skybox,envirounmentMap;
    bool indirectLight;
    std::vector<DirectionalLight> directional;
    std::vector<PointLight> points;
    std::vector<SpotLight> spots;
    //CubeMapRenderTarget* envirounmentMap;
    CubeMapRenderTarget* irradianceMap;
    CubeMapRenderTarget* prefilterMap;
    brdf_lut brdf;
    glshaderprogram *envProgram,*irradianceProgram,*prefilterProgram,*precomputeBRDF;
    public:
    SceneLight(bool envLight = false);
    ~SceneLight();
    void setEnvmap(GLuint &envMap);
    void PrecomputeIndirectLighting();
    void addDirectionalLight(DirectionalLight dl);
    void addPointLight(PointLight pl);
    void addSpotLight(SpotLight sl);
    void setLightUniform(glshaderprogram* program);
    void renderSceneLights(glshaderprogram* program);
    void SceneLightKeyBoardFunc(int key);
    friend std::ostream& operator<<(std::ostream &out, SceneLight* s);
};