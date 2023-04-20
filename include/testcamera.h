#ifndef __TEST_CAMERA__
#define __TEST_CAMERA__

#include<scenecamera.h>
#include<scenecamerarig.h>
#include<camera.h>
#include<memory>

void setupProgramTestCamera();
void setupSceneCameraTestCamera(sceneCamera* &scenecam);
void setupSceneCameraRigTestCamera(sceneCamera* scenecam, sceneCameraRig* &scenecamrig);
void initTestCamera();
void renderTestCamera();
void renderCameraRigTestCamera(const sceneCameraRig* scenecamrig);
void uninitTestCamera();

#endif // __TEST_CAMERA__
