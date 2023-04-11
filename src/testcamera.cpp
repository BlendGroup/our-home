#include<iostream>
#include<memory>
#include<glshaderloader.h>
#include<camera.h>
#include<scenecamera.h>
#include<scenecamerarig.h>
#include<errorlog.h>
#include<global.h>

using namespace std;
using namespace vmath;

static glshaderprogram *testCameraProgram;
static struct test_camera_vaos {
	GLuint plane;
	GLuint cube;
} vaos;

static struct test_camera_vbos {
	GLuint plane;
	GLuint cube;
} vbos;

void setupProgramTestCamera(void) {
    try {
		testCameraProgram = new glshaderprogram({"src/shaders/testcamera.vert", "src/shaders/testcamera.frag"});
	} catch(string errorString) {
		throwErr(errorString);
	}
}

void setupSceneCameraTestCamera(unique_ptr<sceneCamera> &scenecam) {
	PathDescriptor path;
	path.positionKeyFrames.push_back(vec3(0.0f, 20.0f, 35.0f));
    path.positionKeyFrames.push_back(vec3(5.0f, 15.0f, 15.0f));
    path.positionKeyFrames.push_back(vec3(-5.0f, 13.0f, 5.0f));
    path.positionKeyFrames.push_back(vec3(-10.0f, 14.0f, 8.0f));
    path.positionKeyFrames.push_back(vec3(-15.0f, 15.0f, 10.0f));

	path.frontKeyFrames.push_back(vec3(0.0f, -1.0f, 0.0f));
    path.frontKeyFrames.push_back(vec3(10.0f, -1.0f, 10.0f));
    path.frontKeyFrames.push_back(vec3(0.0f, -1.0f, 5.0f));
    path.frontKeyFrames.push_back(vec3(-8.0f, -1.0f, 5.0f));
    path.frontKeyFrames.push_back(vec3(10.0f, 1.0f, 10.0f));

	scenecam.reset(new sceneCamera(&path));
}

void setupSceneCameraRigTestCamera(unique_ptr<sceneCamera> &scenecam, unique_ptr<sceneCameraRig> &scenecamrig) {
	scenecamrig.reset(new sceneCameraRig(scenecam.get()));
	scenecamrig->setRenderPath(true);
	scenecamrig->setRenderPathPoints(true);
	scenecamrig->setRenderFront(true);
	scenecamrig->setRenderFrontPoints(true);
	scenecamrig->setRenderPathToFront(true);
}

void initTestCamera() {
	/* geometry */
	const float planeVerts[] = {
		1.0f, 0.0f, -1.0f,
	   -1.0f, 0.0f, -1.0f,
	    1.0f, 0.0f,  1.0f,

	    1.0f, 0.0f,  1.0f,
	   -1.0f, 0.0f, -1.0f,
	   -1.0f, 0.0f,  1.0f
	};
	glCreateVertexArrays(1, &vaos.plane);
	glBindVertexArray(vaos.plane);
	glCreateBuffers(1, &vbos.plane);
	glBindBuffer(GL_ARRAY_BUFFER, vbos.plane);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVerts), planeVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	const float cubeVerts[] = {
		1.0f,  1.0f,  1.0f,
       -1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,

		1.0f, -1.0f,  1.0f,
	   -1.0f,  1.0f,  1.0f,
	   -1.0f, -1.0f,  1.0f,

		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,

	   -1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
	   -1.0f, -1.0f, -1.0f,

	   -1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

	   -1.0f,  1.0f, -1.0f,
	   -1.0f, -1.0f, -1.0f,
	   -1.0f,  1.0f,  1.0f,

	   -1.0f,  1.0f,  1.0f,
	   -1.0f, -1.0f, -1.0f,
	   -1.0f, -1.0f,  1.0f,

		1.0f,  1.0f, -1.0f,
	   -1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,

		1.0f,  1.0f,  1.0f,
	   -1.0f,  1.0f, -1.0f,
	   -1.0f,  1.0f,  1.0f,

	   -1.0f, -1.0f,  1.0f,
	   -1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		
		1.0f, -1.0f,  1.0f,
	   -1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f
	};
	glCreateVertexArrays(1, &vaos.cube);
	glBindVertexArray(vaos.cube);
	glCreateBuffers(1, &vbos.cube);
	glBindBuffer(GL_ARRAY_BUFFER, vbos.cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerts), cubeVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

void renderTestCamera(const camera *camera) {
	if(camera == NULL) {
		throwErr("Camera Not Inititalized Correctly");
	}

	testCameraProgram->use();
	glUniformMatrix4fv(1, 1, GL_FALSE, camera->matrix());
	glUniformMatrix4fv(2, 1, GL_FALSE, programglobal::perspective);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUniform1i(3, GL_TRUE);  // isPlane = true

	glUniformMatrix4fv(0, 1, GL_FALSE, translate(0.0f, -1.0f, 0.0f) * scale(vec3(50.0f, 0.0f, 50.0f)));
	glBindVertexArray(vaos.plane);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUniform1i(3, GL_FALSE);  // isPlane = false
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniformMatrix4fv(0, 1, GL_FALSE, translate(10.0f, 0.0f, 10.0f) * rotate(36.6f, 0.0f, 1.0f, 0.0f));
	glBindVertexArray(vaos.cube);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glUniformMatrix4fv(0, 1, GL_FALSE, translate(-15.0f, 0.0f, -19.0f) * rotate(-25.0f, 0.0f, 1.0f, 0.0f));
	glBindVertexArray(vaos.cube);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glUniformMatrix4fv(0, 1, GL_FALSE, translate(-21.0f, 0.0f, 7.0f) * rotate(53.2f, 0.0f, 1.0f, 0.0f));
	glBindVertexArray(vaos.cube);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glUniformMatrix4fv(0, 1, GL_FALSE, translate(-13.0f, 0.0f, -8.0f) * rotate(48.8f, 0.0f, 1.0f, 0.0f));
	glBindVertexArray(vaos.cube);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glUniformMatrix4fv(0, 1, GL_FALSE, translate(3.0f, 0.0f, 18.0f) * rotate(129.7f, 0.0f, 1.0f, 0.0f));
	glBindVertexArray(vaos.cube);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glUniformMatrix4fv(0, 1, GL_FALSE, translate(23.0f, 0.0f, -14.0f) * rotate(13.4f, 0.0f, 1.0f, 0.0f));
	glBindVertexArray(vaos.cube);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glUniformMatrix4fv(0, 1, GL_FALSE, translate(0.0f, 0.0f, 0.0f));
	glBindVertexArray(vaos.cube);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void renderCameraRigTestCamera(const camera *camera, const unique_ptr<sceneCameraRig> &scenecamrig) {
	scenecamrig->render(camera);
}

void uninitTestCamera(void) {
	if(vbos.cube) {
		glDeleteBuffers(1, &vbos.cube);
		vbos.cube = 0;
	}
	if(vaos.cube) {
		glDeleteVertexArrays(1, &vaos.cube);
		vaos.cube = 0;
	}
	if(vbos.plane) {
		glDeleteBuffers(1, &vbos.plane);
		vbos.plane = 0;
	}
	if(vaos.plane) {
		glDeleteVertexArrays(1, &vaos.plane);
		vaos.plane = 0;
	}
	if(testCameraProgram) {
		delete testCameraProgram;
	}
}
