#include <cstdlib>
#include<randomgen.h>

using namespace vmath;
using namespace std;

randomgenerator::randomgenerator() {
    engine = new default_random_engine();
    dist = new uniform_real_distribution<float>(0.0f, 1.0f);
    srand(2345);
}

// float randomgenerator::getRandomFloat(float min, float max) {
//     return (*this->dist)(engine) * (max - min) + min;
// }

float randomgenerator::getRandomFloat(float min, float max) {
    return ((float)rand()/RAND_MAX) * (max - min) + min;
}

vec1 randomgenerator::getRandomVector1(float min, float max) {
    return vec1(getRandomFloat(min, max));
}

vec2 randomgenerator::getRandomVector2(float min, float max) {
    return vec2(
        getRandomFloat(min, max),
        getRandomFloat(min, max)
    );
}

vec3 randomgenerator::getRandomVector3(float min, float max) {
    return vec3(
        getRandomFloat(min, max),
        getRandomFloat(min, max),
        getRandomFloat(min, max)
    );
}

vec4 randomgenerator::getRandomVector4(float min, float max) {
    return vec4(
        getRandomFloat(min, max),
        getRandomFloat(min, max),
        getRandomFloat(min, max),
        getRandomFloat(min, max)
    );
}

randomgenerator::~randomgenerator() {
    delete engine;
    delete dist;
}