#ifndef __TEST_CAMERA__
#define __TEST_CAMERA__

#include<scenecamera.h>
#include<scenecamerarig.h>
#include<camera.h>
#include<memory>

void setupProgramTestCamera();
void setupSceneCameraTestCamera(std::unique_ptr<sceneCamera> &scenecam);
void setupSceneCameraRigTestCamera(std::unique_ptr<sceneCamera> &scenecam, std::unique_ptr<sceneCameraRig> &scenecamrig);
void initTestCamera();
void renderTestCamera(const camera *camera);
void renderCameraRigTestCamera(const camera *camera, const std::unique_ptr<sceneCameraRig> &scenecamrig);
void uninitTestCamera();

#endif // __TEST_CAMERA__
