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
    bool isRenderPath, isRenderFront, isRenderPathToFront;
    float t;

    void loadGeometry(void);

public:
    sceneCameraRig(sceneCamera *camera);
    ~sceneCameraRig();
    void render() const;
    void updateT(float speed);
    void setRenderPath(bool setting);
    void setRenderPathPoints(bool setting);
    void setRenderFront(bool setting);
    void setRenderFrontPoints(bool setting);
    void setRenderPathToFront(bool setting);
};

#endif // __SCENE_CAMERA_RIG__
