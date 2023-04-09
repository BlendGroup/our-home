#ifndef __TEST_CAMERA__
#define __TEST_CAMERA__

#include"scenecamerarig.h"
#include"scenecamera.h"
#include"camera.h"

void setupProgramTestCamera();
void setupSceneCameraTestCamera(sceneCamera* &scenecam);
void setupSceneCameraRigTestCamera(sceneCamera* &scenecam, sceneCameraRig* &scenecamrig);
void initTestCamera();
void renderTestCamera(camera* cam);
void uninitTestCamera();

#endif // __TEST_CAMERA__
