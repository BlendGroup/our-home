#ifndef __SCENE_CAMERA_H__
#define __SCENE_CAMERA_H__

#include<vector>
#include<vmath.h>
#include<interpolators.h>
#include<errorlog.h>
#include<camera.h>
#include<ostream>

class sceneCamera : public camera
{
private:
    BsplineInterpolator *m_bspPositions;
    BsplineInterpolator *m_bspFront;
    float t;
	friend class sceneCameraRig;

public:
    sceneCamera(const std::vector<vmath::vec3> &positionKeyFrames, const std::vector<vmath::vec3> &frontKeyFrames);
    ~sceneCamera();
	float getDistanceOnSpline() const;
    void updateT(float speed);
	void setT(float t);
	float getT();
	void resetT();
	vmath::mat4 matrix() const override;
	vmath::mat4 matrixYFlippedOnPlane(float planey) const override;
	vmath::vec3 position() const override;
	friend std::ostream& operator<< (std::ostream &out, const sceneCamera &t);
	friend std::ostream& operator<< (std::ostream &out, const sceneCamera *t);
};

#endif // __SCENE_CAMERA_H__
