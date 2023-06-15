#include<fireflies.h>

#define COLOR_MIX_SPEED 0.5f

using namespace std;
using namespace vmath;

/************************** Flock 'O Fireflies **************************/
Fireflies::Fireflies(size_t maxParticles, const std::vector<vmath::vec3> &pathPoints, const std::vector<vmath::vec3> &colors) {
    this->flock = new Flock(maxParticles);
    this->bspPath = new BsplineInterpolator(pathPoints);
    this->bspColors = new BsplineInterpolator(colors);
    this->colorT = 0.0f;
    this->colorTDirection = true;  // colorT increasing = true, decreasing = false
}

Fireflies::~Fireflies() {
    if(this->bspColors) {
        delete this->bspColors;
        this->bspColors = NULL;
    }
    if(this->bspPath) {
        delete this->bspPath;
        this->bspPath = NULL;
    }
    if(this->flock) {
        delete this->flock;
        this->flock = NULL;
    }
}

void Fireflies::update(void) { 
    if(this->colorTDirection)
        this->colorT += COLOR_MIX_SPEED * programglobal::deltaTime;
    else
        this->colorT -= COLOR_MIX_SPEED * programglobal::deltaTime;
    
    if(this->colorT > 1.0f) {
        this->colorT = 1.0f;
        this->colorTDirection = false;
    }
    else if(this->colorT < 0.0f) {
        this->colorT = 0.0f;
        this->colorTDirection = true;
    }
    this->flock->update();
}

void Fireflies::renderAsSpheres(float positionT, float scale) {
    vec3 position = this->bspPath->interpolate(positionT);
    vec3 color = this->bspColors->interpolate(this->colorT);
    this->flock->renderAsSpheres(translate(position), vec4(color, 1.0f), vec4(color, 1.0f), scale);
}

void Fireflies::renderAttractorAsQuad(float positionT, float scale) {
    vec3 position = this->bspPath->interpolate(positionT);
    vec3 color = this->bspColors->interpolate(this->colorT);
    this->flock->renderAttractorAsQuad(translate(position), vec4(color, 1.0f), vec4(color, 1.0f), scale);
}

BsplineInterpolator *Fireflies::getPath(void) {
    return this->bspPath;
}
/********************************* EOF **********************************/
