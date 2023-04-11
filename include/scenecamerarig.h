#ifndef __SCENE_CAMERA_RIG__
#define __SCENE_CAMERA_RIG__

#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include "../include/vmath.h"
#include "../include/glshaderloader.h"
#include "../include/scenecamera.h"
#include "../include/interpolators.h"
#include "../include/splinerenderer.h"
#include "../include/global.h"

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
    void render(const camera* &camera) const;
    void updateT(float speed);
    void setRenderPath(bool setting);
    void setRenderPathPoints(bool setting);
    void setRenderFront(bool setting);
    void setRenderFrontPoints(bool setting);
    void setRenderPathToFront(bool setting);
};

#endif // __SCENE_CAMERA_RIG__
