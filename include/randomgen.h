#ifndef __RANDOM__
#define __RANDOM__

#include<vmath.h>
#include<random>

class randomgenerator {
private:
    std::default_random_engine *engine;
    std::uniform_real_distribution<float> *dist;
public:
    randomgenerator(void);
    float getRandomFloat(float min, float max);
    vmath::vec1 getRandomVector1(float min, float max);
    vmath::vec2 getRandomVector2(float min, float max);
    vmath::vec3 getRandomVector3(float min, float max);
    vmath::vec4 getRandomVector4(float min, float max);
    ~randomgenerator();
};

#endif