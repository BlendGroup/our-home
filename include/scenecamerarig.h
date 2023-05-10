#ifndef __SCENE_CAMERA_RIG__
#define __SCENE_CAMERA_RIG__

#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <vmath.h>
#include <glshaderloader.h>
#include <scenecamera.h>
#include <interpolators.h>
#include <splinerenderer.h>
#include <global.h>

class sceneCameraRig
{
private:
    sceneCamera *mountCamera;
    SplineRenderer *pathRenderer;
    SplineRenderer *frontRenderer;
    glshaderprogram *program;
    GLuint vaoPoint, vaoPathToFront;
    GLuint vboPoint, vboPathToFront;
    bool isRenderPath, isRenderFront, isRenderPathToFront, isRenderPathPoints, isRenderFrontPoints;
    float scalingFactor;
	std::vector<vmath::vec3> positionKeyFrames;
	std::vector<vmath::vec3> frontKeyFrames;
    int selectedPathPoint;
    int selectedFrontPoint;

	void loadGeometry(void);

public:
    sceneCameraRig(std::vector<vmath::vec3> positionKeyFrames, std::vector<vmath::vec3> frontKeyFrames);
    ~sceneCameraRig();
    void render() const;
    void updateT(float speed);
	void resetT();
    void setRenderPath(bool setting);
    void setRenderPathPoints(bool setting);
    void setRenderFront(bool setting);
    void setRenderFrontPoints(bool setting);
    void setRenderPathToFront(bool setting);
	void setScalingFactor(float scalingFactor);
	sceneCamera* getCamera();
	void keyboardfunc(int key);
};

#endif // __SCENE_CAMERA_RIG__
