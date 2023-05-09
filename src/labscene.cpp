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
static glmodel* modelAstro;
static glshaderprogram* renderModelDebug;
static glshaderprogram* renderModelAnimDebug;
static BsplineInterpolator* bspRobot;

#ifdef DEBUG
static modelplacer* astroPlacer;
static SplineRenderer* splineRender;
int selectedPoint = 0;
#endif
static vector<vec3> robotSpline = {
	vec3(2.8f, -1.067f, -0.18f),
	vec3(0.9f, -1.067f, -0.21f),
	vec3(0.2f, -1.067f, 0.6f),
	vec3(-2.0f, -1.067f, 0.6f),
	vec3(-2.5f, -1.067f, 1.8f)
};

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
		vec3(-1.17f, 0.06f, -1.1f),
		vec3(-2.9f, -0.2f, -0.51f),
		vec3(-3.49f, -0.56f, 0.05f)
	};
    
	vector<vec3> frontKeyFrames = {
		vec3(-1.96f, -0.35f, -1.65f),
		vec3(-1.74f, -0.3f, -1.29f),
		vec3(-1.38f, -0.47f, -1.48f),
		vec3(-1.26f, -0.4f, -1.5f),
		vec3(-0.8f, -0.28f, -1.23f),
		vec3(-0.47f, -0.26f, -0.37f),
		vec3(-1.67f, -0.42f, 0.45f),
		vec3(-3.19f, -0.36f, 0.96f),
		vec3(-3.49f, -0.27f, 2.02f)
	};

	return new sceneCamera(positionKeyFrames, frontKeyFrames);
}

void labscene::init() {
	modelLab = new glmodel("resources/models/spaceship/SpaceLab.fbx", 0, false);
	modelMug = new glmodel("resources/models/mug/mug.glb", 0, false);
	modelRobot = new glmodel("resources/models/robot/robot.fbx", 0, false);
	modelAstro = new glmodel("resources/models/astronaut/MCAnim.fbx", 0, false);
	bspRobot = new BsplineInterpolator(robotSpline);
#ifdef DEBUG
	splineRender = new SplineRenderer(bspRobot);
	splineRender->setRenderPoints(true);
	//Astronaut: vec3(-3.41f, -1.39f, 2.03f), vec3(0f, 0f, 0f), 0.00889994f
	astroPlacer = new modelplacer();
#endif
}

static float t = 0.0f;

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
	vec3 position = bspRobot->interpolate(t);
	vec3 front = bspRobot->interpolate(t + 0.01f);
	glUniformMatrix4fv(renderModelAnimDebug->getUniformLocation("mMat"), 1, GL_FALSE, translate(position) * targetat(position, front, vec3(0.0f, 1.0f, 0.0f)) * scale(0.042f));
	modelRobot->setBoneMatrixUniform(renderModelAnimDebug->getUniformLocation("bMat[0]"), 0);
	modelRobot->draw(renderModelAnimDebug);
	modelRobot->update(0.01f, 0);


	renderModelAnimDebug->use();
	glUniformMatrix4fv(renderModelAnimDebug->getUniformLocation("pMat"), 1, GL_FALSE, programglobal::perspective);
	glUniformMatrix4fv(renderModelAnimDebug->getUniformLocation("vMat"), 1, GL_FALSE, programglobal::currentCamera->matrix());
	glUniformMatrix4fv(renderModelAnimDebug->getUniformLocation("mMat"), 1, GL_FALSE, translate(-3.41f, -1.39f, 2.03f) * scale(0.00889994f));
	modelAstro->setBoneMatrixUniform(renderModelAnimDebug->getUniformLocation("bMat[0]"), 0);
	modelAstro->draw(renderModelAnimDebug);
	modelAstro->update(0.01f, 0);

#ifdef DEBUG
	splineRender->render(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), selectedPoint, 0.01f);
#endif

	t += 0.0006f;
}

void labscene::uninit() {
	delete modelLab;
	delete modelMug;
	delete modelRobot;
}

void splineKeyboardFunc(int key) {
	bool updatePos;
	switch(key) {
		//Path Point Select
	case XK_n:
		selectedPoint = (selectedPoint == 0) ? robotSpline.size() - 1 : (selectedPoint - 1) % robotSpline.size();
		updatePos = true;
		break;
	case XK_m:
		selectedPoint = (selectedPoint + 1) % robotSpline.size();
		updatePos = true;
		break;
	//Path Point Move Z
	case XK_i:
		robotSpline[selectedPoint][2] += 0.1f;
		updatePos = true;
		break;
	case XK_k:
		robotSpline[selectedPoint][2] -= 0.1f;
		updatePos = true;
		break;
	//Path Point Move X
	case XK_l:
		robotSpline[selectedPoint][0] += 0.1f;
		updatePos = true;
		break;
	case XK_j:
		robotSpline[selectedPoint][0] -= 0.1f;
		updatePos = true;
		break;
	//Path Point Move Y
	case XK_o:
		robotSpline[selectedPoint][1] += 0.1f;
		updatePos = true;
		break;
	case XK_u:
		robotSpline[selectedPoint][1] -= 0.1f;
		updatePos = true;
		break;
	//Add/Remove Path Points
	case XK_bracketright:
		robotSpline.insert(robotSpline.begin() + selectedPoint + 1, vec3(0.0f, 0.0f, 0.0f));
		updatePos = true;
		break;
	case XK_bracketleft:
		robotSpline.erase(robotSpline.begin() + selectedPoint);
		selectedPoint = selectedPoint % robotSpline.size();
		updatePos = true;
		break;
	case XK_Left:
		t -= 0.01f;
		t = std::max(t, 0.0f);
		break;
	case XK_Right:
		t += 0.01f;
		t = std::min(t, 1.0f);
		break;
	}
	if(updatePos) {
		delete bspRobot;
		bspRobot = new BsplineInterpolator(robotSpline);
		delete splineRender;
		splineRender = new SplineRenderer(bspRobot);
		splineRender->setRenderPoints(true);
	}
}

void labscene::keyboardfunc(int key) {
#ifdef DEBUG
	bool updatePos = false;
	// astroPlacer->keyboardfunc(key);
	// splineKeyboardFunc(key);
	switch(key) {
	case XK_Tab:
		cout<<astroPlacer<<endl;
		for(int i = 0; i < robotSpline.size(); i++) {
			cout<<"\t"<<robotSpline[i]<<",\n";
		}
		cout<<"\b"<<endl;
		break;
	}
#endif
}