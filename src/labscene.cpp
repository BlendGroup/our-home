#define DEBUG
#include<scenes/lab.h>
#include<glmodelloader.h>
#include<glshaderloader.h>
#include<iostream>
#include<global.h>
#include<vmath.h>
#include<scenecamera.h>
#include<modelplacer.h>
#include<X11/keysym.h>
#include<interpolators.h>
#include<splinerenderer.h>

using namespace std;
using namespace vmath;

static glmodel* modelLab;
static glmodel* modelMug;
static glmodel* modelRobot;
static glshaderprogram* renderModelDebug;
static glshaderprogram* renderModelAnimDebug;
static BsplineInterpolator* bspRobot;

#ifdef DEBUG
static modelplacer* robotPlacer;
static SplineRenderer* splineRender;
static vector<vec3> robotSpline = {
	vec3(0.0f, 0.0f, 0.0f),
	vec3(1.0f, 0.0f, 0.0f),
	vec3(2.0f, 0.0f, 0.0f),
	vec3(3.0f, 0.0f, 0.0f)
};
int selectedPoint = 0;
#endif

void labscene::setupProgram() {
	//Debug Program/////////////////
	renderModelDebug = new glshaderprogram({"shaders/debug/basictex.vert", "shaders/debug/basictex.frag"});
	renderModelAnimDebug = new glshaderprogram({"shaders/debug/basicanimtex.vert", "shaders/debug/basictex.frag"});
	////////////////////////////////
}

sceneCamera* labscene::setupCamera() {
	vector<vec3> positionKeyFrames = {
		vec3(-1.96f, -0.22f, -1.11f),
		vec3(-1.82f, -0.08f, -0.91f),
		vec3(-1.71f, -0.12f, -1.07f),
		vec3(-1.63f, -0.3f, -1.7f),
		vec3(-1.15f, -0.05f, -1.81f),
		vec3(-0.82f, 0.28f, -1.85f),
		vec3(-1.17f, -2.98f, -1.1f),
		vec3(-2.9f, -0.2f, -0.51f),
		vec3(-3.49f, -0.56f, 0.05f),
	};
    
	vector<vec3> frontKeyFrames = {
		vec3(-1.96f, -0.35f, -1.65f),
		vec3(-1.74f, -0.3f, -1.29f),
		vec3(-1.38f, -0.47f, -1.48f),
		vec3(-1.26f, -0.4f, -1.5f),
		vec3(-0.8f, -0.28f, -1.23f),
		vec3(-0.47f, -0.26f, -0.37f),
		vec3(-1.67f, -0.42f, 0.45f),
		vec3(-3.19f, -0.36f, 0.959999f),
		vec3(-3.49f, -0.27f, 2.02f),
	};

	return new sceneCamera(positionKeyFrames, frontKeyFrames);
}

void labscene::init() {
	modelLab = new glmodel("resources/models/spaceship/SpaceLab.fbx", 0, false);
	modelMug = new glmodel("resources/models/mug/mug.glb", 0, false);
	modelRobot = new glmodel("resources/models/robot/robot.fbx", 0, false);
	bspRobot = new BsplineInterpolator(robotSpline);
#ifdef DEBUG
	splineRender = new SplineRenderer(bspRobot);
	splineRender->setRenderPoints(true);
	robotPlacer = new modelplacer(vec3(0.115f, -1.067f, 0.611f), vec3(0.0f, 0.0f, 0.0f), 0.042f);
#endif
}

void labscene::render() {
	renderModelDebug->use();
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(renderModelDebug->getUniformLocation("mMat"), 1, GL_FALSE, mat4::identity());
	modelLab->draw(renderModelDebug);

	glUniformMatrix4fv(renderModelDebug->getUniformLocation("mMat"), 1, GL_FALSE, translate(-1.3f,-0.41f,-1.5f) * scale(0.08f,0.08f,0.08f));
	modelMug->draw(renderModelDebug);

	renderModelAnimDebug->use();
	glUniformMatrix4fv(renderModelAnimDebug->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(renderModelAnimDebug->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(renderModelAnimDebug->getUniformLocation("mMat"), 1, GL_FALSE, translate(1.705f, -1.067f, -0.179999f) * rotate(-58.0f, 0.0f, 1.0f, 0.0f) * scale(0.042f));
	modelRobot->setBoneMatrixUniform(renderModelAnimDebug->getUniformLocation("bMat[0]"), 0);
	modelRobot->draw(renderModelAnimDebug);
	modelRobot->update(0.01f, 0);

#ifdef DEBUG
	splineRender->render(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), selectedPoint, 0.01f);
#endif
}

void labscene::uninit() {
	delete modelLab;
	delete modelMug;
	delete modelRobot;
}

void labscene::keyboardfunc(int key) {
#ifdef DEBUG
	// robotPlacer->keyboardfunc(key);
	switch(key) {
	case XK_Tab:
		cout<<robotPlacer<<endl;
		break;
	//Path Point Select
	case XK_n:
		selectedPoint = (selectedPoint == 0) ? robotSpline.size() - 1 : (selectedPoint - 1) % robotSpline.size();
	case XK_m:
		selectedPoint = (selectedPoint + 1) % robotSpline.size();
	//Path Point Move Z
	case XK_i:
		robotSpline[selectedPoint][2] += 0.1f;
	case XK_k:
		robotSpline[selectedPoint][2] -= 0.1f;
	//Path Point Move X
	case XK_l:
		robotSpline[selectedPoint][0] += 0.1f;
	case XK_j:
		robotSpline[selectedPoint][0] -= 0.1f;
	//Path Point Move Y
	case XK_o:
		robotSpline[selectedPoint][1] += 0.1f;
	case XK_u:
		robotSpline[selectedPoint][1] -= 0.1f;
	//Add/Remove Path Points
	case XK_bracketleft:
		robotSpline.insert(robotSpline.begin() + selectedPoint + 1, vec3(0.0f, 0.0f, 0.0f));
	case XK_bracketright:
		robotSpline.erase(robotSpline.begin() + selectedPoint);
		selectedPoint = selectedPoint % robotSpline.size();

		delete bspRobot;
		bspRobot = new BsplineInterpolator(robotSpline);
		delete splineRender;
		splineRender = new SplineRenderer(bspRobot);
		splineRender->setRenderPoints(true);
	}
#endif
}