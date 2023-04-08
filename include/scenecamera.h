#ifndef __SCENE_CAMERA_H__
#define __SCENE_CAMERA_H__

#include <vector>
#include "../include/vmath.h"
#include "../include/interpolators.h"
#include "../include/errorlog.h"

struct PathDescriptor
{
    std::vector<vmath::vec3> positionKeyFrames;
    std::vector<vmath::vec3> frontKeyFrames;
};

class SceneCamera
{
private:
    BsplineInterpolator *m_bspPositions;
    BsplineInterpolator *m_bspFront;
    const PathDescriptor *m_pdesc;
public:
    SceneCamera(const PathDescriptor *pdesc);
    ~SceneCamera();
    float getDistanceOnSpline(const float t) const;
    vmath::mat4 matrix(const float t) const;
};

#endif // __SCENE_CAMERA_H__
