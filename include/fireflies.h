#ifndef __FIREFLIES__
#define __FIREFLIES__

#include<vector>
#include<flock.h>
#include<interpolators.h>
#include<vmath.h>
#include<global.h>

class Fireflies {
private:
    Flock *flock;
    BsplineInterpolator *bspPath;
    BsplineInterpolator *bspColors;
    float colorT;
    bool colorTDirection;
public:
    Fireflies(size_t maxParticles, const std::vector<vmath::vec3> &pathPoints, const std::vector<vmath::vec3> &colors);
    ~Fireflies();
    void update(void);
    void renderAsSpheres(float positionT, float scale);
    void renderAttractorAsQuad(float positionT, float scale);
    BsplineInterpolator *getPath(void);
};

#endif // __FIREFLIES__
