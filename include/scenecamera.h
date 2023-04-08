#ifndef __SCENE_CAMERA_H__
#define __SCENE_CAMERA_H__

#include <vector>
#include "../include/vmath.h"
#include "../include/interpolators.h"
#include "../include/errorlog.h"
#include "../include/camera.h"

struct PathDescriptor
{
    std::vector<vmath::vec3> positionKeyFrames;
    std::vector<vmath::vec3> frontKeyFrames;
};

class sceneCamera : public camera
{
private:
    BsplineInterpolator *m_bspPositions;
    BsplineInterpolator *m_bspFront;
    const PathDescriptor *m_pdesc;
	float t;
public:
    sceneCamera(const PathDescriptor *pdesc);
    ~sceneCamera();
    float getDistanceOnSpline() const;
	void updateT(float speed);
    vmath::mat4 matrix() const override;
};

#endif // __SCENE_CAMERA_H__
