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
    // const PathDescriptor *m_pdesc;
	std::vector<vmath::vec3> positionKeyFrames;
	std::vector<vmath::vec3> frontKeyFrames;
    float t;
    friend class sceneCameraRig;

public:
    sceneCamera(const std::vector<vmath::vec3> &positionKeyFrames, const std::vector<vmath::vec3> &frontKeyFrames);
    ~sceneCamera();
	std::vector<vmath::vec3>& getPointerToPositionKeyFrame();
	std::vector<vmath::vec3>& getPointerToFrontKeyFrame();
    void reinitializePositionSpline();
    void reinitializeFrontSpline();
	float getDistanceOnSpline() const;
    void updateT(float speed);
	void resetT();
	vmath::mat4 matrix() const override;
	vmath::vec3 position() const override;
	friend std::ostream& operator<< (std::ostream &out, const sceneCamera &t);
	friend std::ostream& operator<< (std::ostream &out, const sceneCamera *t);
};

#endif // __SCENE_CAMERA_H__
