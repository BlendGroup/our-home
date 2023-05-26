#include<scenes/lab.h>
#include<glmodelloader.h>
#include<glshaderloader.h>
#include<gltextureloader.h>
#include<iostream>
#include<unordered_map>
#include<global.h>
#include<vmath.h>
#include<scenecamera.h>
#include<scenecamerarig.h>
#include<modelplacer.h>
#include<X11/keysym.h>
#include<interpolators.h>
#include<splineadjuster.h>
#include<CubeMapRenderTarget.h>
#include<glLight.h>
#include<assimp/postprocess.h>
#include<audio.h>

using namespace std;
using namespace vmath;

#define CUBEMAP_SIZE 2048

enum EVENTS {
	DOOR_ANIM = 0,
	ROBOT_ANIM,
	BKGND_MUSIC_PLAY,
	CAMERA_MOVE,
	CROSSFADE,

//Dont Add	
	NUM_EVENTS
};

static bool eventManager[NUM_EVENTS];

static sceneCamera* camera1;

static glmodel* modelLab;
static glmodel* modelDoor;
static glmodel* modelMug;
static glmodel* modelRobot;
static glmodel* modelAstro;
static glmodel* modelBLEND;

static BsplineInterpolator* bspRobot;

static CubeMapRenderTarget* envMapper;

static SceneLight* sceneLightManager;

static glshaderprogram* programStaticPBR;
static glshaderprogram* programDynamicPBR;
static glshaderprogram* programLight;
static glshaderprogram* programSkybox;
static glshaderprogram* programColor;
static glshaderprogram* programHologram;
static glshaderprogram* programCrossfade;

static GLuint skybox_vao,vbo;

static audioplayer *playerBkgnd;
static audioplayer *playerRobotThump;

#ifdef DEBUG
static modelplacer* doorPlacer;
static sceneCameraRig* cameraRig;
static SplineAdjuster* robotSpline;
#endif

static GLfloat robotT		= 0.01f;
static GLfloat astonautT	= 0.0f;
static GLfloat cameraT		= 0.0f;
static GLfloat doorT		= 0.0f;
static GLfloat hologramT	= 0.0f;
static GLfloat crossT		= 0.0f;

extern GLuint texTitleSceneFinal;

void labscene::setupProgram() {
	try {
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
		programLight = new glshaderprogram({"shaders/debug/lightSrc.vert", "shaders/debug/lightSrc.frag"});
		programSkybox = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/debug/rendercubemap.frag"});
		programColor = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
		programHologram = new glshaderprogram({"shaders/hologram/hologram.vert","shaders/hologram/hologram.frag"});
		programCrossfade = new glshaderprogram({"shaders/fsquad.vert", "shaders/crossfade.frag"});
	} catch(string errorString)  {
		throwErr(errorString);
	}
}

void labscene::setupCamera() {
	vector<vec3> positionKeyFrames = {
		vec3(-1.96f, -0.27f, -1.13f),
		vec3(-1.94f, -0.32f, -0.97f),
		vec3(-1.89f, -0.37f, -1.22f),
		vec3(-1.84f, -0.33f, -1.65f),
		vec3(-1.47f, -0.15f, -1.82f),
		vec3(-0.82f, 0.28f, -1.85f),
		vec3(-1.17f, 0.06f, -1.1f),
		vec3(-2.9f, -0.2f, -0.51f),
		vec3(-3.49f, -0.56f, 0.05f)
	};
    
	vector<vec3> frontKeyFrames = {
		vec3(-1.96f, -0.27f, -1.65f),
		vec3(-1.84f, -0.4f, -1.42f),
		vec3(-1.51f, -0.4f, -1.37f),
		vec3(-1.36f, -0.38f, -1.43f),
		vec3(-0.51f, -0.28f, -0.98f),
		vec3(-0.21f, -0.26f, -0.37f),
		vec3(-1.67f, -0.42f, 0.45f),
		vec3(-3.19f, -0.36f, 0.96f),
		vec3(-3.49f, -0.27f, 2.02f)
	};

	camera1 = new sceneCamera(positionKeyFrames, frontKeyFrames);

	cameraRig = new sceneCameraRig(camera1);
	cameraRig->setRenderFront(true);
	cameraRig->setRenderFrontPoints(true);
	cameraRig->setRenderPath(true);
	cameraRig->setRenderPathPoints(true);
	cameraRig->setRenderPathToFront(true);
	cameraRig->setScalingFactor(0.01f);
}

void labscene::init() {
	for(int i = 0; i < NUM_EVENTS; i++) {
		eventManager[i] = false;
	}
	eventManager[CROSSFADE] = true;
	
	playerBkgnd = new audioplayer("resources/audio/TheLegendOfKai.wav");
	playerRobotThump = new audioplayer("resources/audio/MetallicThumps.wav");

	modelLab = new glmodel("resources/models/spaceship/SpaceLab.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelDoor = new glmodel("resources/models/spaceship/door.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelMug = new glmodel("resources/models/mug/mug.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelRobot = new glmodel("resources/models/robot/robot.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelAstro = new glmodel("resources/models/astronaut/MCAnim.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelBLEND = new glmodel("resources/models/blendlogo/BLEND.glb",aiProcessPreset_TargetRealtime_Quality,false);

	bspRobot = new BsplineInterpolator({
		vec3(1.6f, -1.067f, -1.28f),
		vec3(1.1f, -1.067f, -0.21f),
		vec3(0.2f, -1.067f, 0.6f),
		vec3(-2.0f, -1.067f, 0.6f),
		vec3(-2.5f, -1.067f, 1.8f)
	});
#ifdef DEBUG
	robotSpline = new SplineAdjuster(bspRobot);
	robotSpline->setRenderPath(true);
	robotSpline->setRenderPoints(true);
	robotSpline->setScalingFactor(0.01f);
#endif

	envMapper = new CubeMapRenderTarget(CUBEMAP_SIZE, CUBEMAP_SIZE, false);
	envMapper->setPosition(vec3(0.0f, 0.0f, 0.0f));
	
	sceneLightManager = new SceneLight(true);
	sceneLightManager->addDirectionalLight(DirectionalLight(vec3(0.1f),10.0f,vec3(0.0,0.0,-1.0f)));
	sceneLightManager->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(-5.0f,5.0f,-5.0f),25.0f));
	sceneLightManager->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(5.0f,5.0f,-5.0f),25.0f));
	sceneLightManager->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(5.0f,5.0f,5.0f),25.0f));
	sceneLightManager->addPointLight(PointLight(vec3(1.0f,1.0f,1.0f),100.0f,vec3(-5.0f,5.0f,5.0f),25.0f));
	sceneLightManager->addSpotLight(SpotLight(vec3(0.0f,1.0f,0.0f),100.0f,vec3(-6.0f,8.0f,3.5f),35.0f,vec3(0.0f,0.0f,-1.0f),30.0f,45.0f));

	float skybox_positions[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	glCreateVertexArrays(1, &skybox_vao);
	glBindVertexArray(skybox_vao);
	glCreateBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_positions), skybox_positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

#ifdef DEBUG
	doorPlacer = new modelplacer(vec3(-1.38f, -0.41f, -1.45f), vec3(0.0f, 0.0f, 0.0f), 0.08f);
#endif
	
	programStaticPBR = new glshaderprogram({"shaders/pbr.vert", "shaders/pbrMain.frag"});
	glBindFramebuffer(GL_FRAMEBUFFER, envMapper->FBO);
	glViewport(0, 0, envMapper->width, envMapper->height);

	for(int side = 0; side < 6; side++){
		glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_CUBE_MAP_POSITIVE_X + side,envMapper->cubemap_texture,0); 
		glClearBufferfv(GL_COLOR, 0, vec4(0.1f, 0.1f, 0.1f, 1.0f));
		glClearBufferfv(GL_DEPTH, 0, vec1(1.0f));

		programStaticPBR->use();
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"),1,GL_FALSE,envMapper->projection);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"),1,GL_FALSE,envMapper->view[side]);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
		glUniform3fv(programStaticPBR->getUniformLocation("viewPos"),1,envMapper->position);
		// Lights data
		glUniform1i(programStaticPBR->getUniformLocation("specularGloss"),false);
		sceneLightManager->setLightUniform(programStaticPBR, false);
		modelLab->draw(programStaticPBR,1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	sceneLightManager->setEnvmap(envMapper->cubemap_texture);
	sceneLightManager->PrecomputeIndirectLighting();
}

void labscene::render() {
	try {
		programStaticPBR->use();
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"),1,GL_FALSE, programglobal::perspective);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"),1,GL_FALSE, programglobal::currentCamera->matrix());
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"),1,GL_FALSE, translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
		glUniform3fv(programStaticPBR->getUniformLocation("viewPos"),1, programglobal::currentCamera->position());
		// Lights data
		glUniform1i(programStaticPBR->getUniformLocation("specularGloss"),false);
		sceneLightManager->setLightUniform(programStaticPBR);
		modelLab->draw(programStaticPBR);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(mix(vec3(-3.3, -0.4f, 2.8f), vec3(-4.62f, -0.4f, 2.8f), doorT)));
		modelDoor->draw(programStaticPBR);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, doorPlacer->getModelMatrix());
		modelMug->draw(programStaticPBR);

		programDynamicPBR->use();
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"), 1,GL_FALSE, programglobal::perspective);
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"), 1,GL_FALSE, programglobal::currentCamera->matrix());
		vec3 position = bspRobot->interpolate(robotT - 0.01f);
		vec3 front = bspRobot->interpolate(robotT);
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(position) * targetat(position, front, vec3(0.0f, 1.0f, 0.0f)) * scale(0.042f));
		glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
		// Lights data
		glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"), true);
		sceneLightManager->setLightUniform(programDynamicPBR);
		modelRobot->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
		modelRobot->draw(programDynamicPBR,1); 

		programDynamicPBR->use();
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"),1,GL_FALSE, programglobal::currentCamera->matrix());
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"),1,GL_FALSE, translate(-3.41f, -1.39f, 2.03f) * scale(1.0f));
		glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"),1, programglobal::currentCamera->position());
		// Lights data
		glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"),false);
		sceneLightManager->setLightUniform(programDynamicPBR);
		modelAstro->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
		modelAstro->draw(programDynamicPBR);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		programHologram->use();
		glUniformMatrix4fv(programHologram->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(programHologram->getUniformLocation("vMat"),1,GL_FALSE, programglobal::currentCamera->matrix());
		//translate(-1.96f, -0.27f, -1.682f) * rotate(13f, 1.0f, 0.0f, 0.0f) * rotate(-50f, 0.0f, 1.0f, 0.0f) * rotate(-1f, 0.0f, 0.0f, 1.0f) * scale(0.11f)
        glUniformMatrix4fv(programHologram->getUniformLocation("mMat"),1,GL_FALSE, translate(-1.96013f, -0.266205f, -1.682f) * scale(0.11f));
		glUniform4fv(programHologram->getUniformLocation("MainColor"),1,vec4(0.0f,0.0f,1.0f,1.0f));
		glUniform4fv(programHologram->getUniformLocation("RimColor"),1,vec4(0.0f,1.0f,1.0f,1.0f));
		glUniform1f(programHologram->getUniformLocation("gTime"), hologramT);
		glUniform1f(programHologram->getUniformLocation("GlitchIntensity"), 1.0f);
		glUniform1f(programHologram->getUniformLocation("GlitchSpeed"), 1.0f);
		glUniform1f(programHologram->getUniformLocation("BarSpeed"),7.0f);
		glUniform1f(programHologram->getUniformLocation("BarDistance"),0.1f);
		glUniform1f(programHologram->getUniformLocation("alpha"), 0.5f);
		glUniform1f(programHologram->getUniformLocation("FlickerSpeed"),5.0f);
		glUniform1f(programHologram->getUniformLocation("RimPower"),5.0f);
		// glUniform1f(programHologram->getUniformLocation("GlowSpeed"),1.0f);
		// glUniform1f(programHologram->getUniformLocation("GlowDistance"),1.0f);
		modelBLEND->draw(programHologram,1,false);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);

		programColor->use();
		glUniformMatrix4fv(programColor->getUniformLocation("mvpMatrix"), 1, GL_FALSE, programglobal::perspective * programglobal::currentCamera->matrix() * translate(-3.45f, -0.3f, 2.828f));
		glUniform4f(programColor->getUniformLocation("color"), 1.0f, 1.0f, 1.0f, 1.0f);
		programglobal::shapeRenderer->renderQuad();

		// render light src
		programLight->use();
		glUniformMatrix4fv(programLight->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
		glUniformMatrix4fv(programLight->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix()); 
		sceneLightManager->renderSceneLights(programLight);

		// render to cubemap test
		programSkybox->use();
		glUniformMatrix4fv(programSkybox->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
		glUniformMatrix4fv(programSkybox->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix());
		glBindVertexArray(skybox_vao);
		glBindTextureUnit(0,envMapper->cubemap_texture);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		if(!eventManager[CAMERA_MOVE]) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			programCrossfade->use();
			glUniform1f(programCrossfade->getUniformLocation("alpha"), 1.0f - crossT);
			glUniform1i(programCrossfade->getUniformLocation("texSampler"), 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texTitleSceneFinal);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glDisable(GL_BLEND);
		}

		if(programglobal::debugMode == CAMERA) {
			cameraRig->render();
		} else if(programglobal::debugMode == SPLINE) {
			robotSpline->render(RED_PINK_COLOR);
		}
	} catch(string errString) {
		throwErr(errString);
	}
}

void labscene::reset() {
	t = 0.0f;
}

void labscene::update() {
	t += programglobal::deltaTime;

	if(crossT >= 1.0f) {
		eventManager[CAMERA_MOVE] = true;
		eventManager[CROSSFADE] = false;
	}
	if(doorT >= 1.0f) {
		cout<<t<<endl;
		playNextScene();
	}
	if(robotT >= 1.0f) {
		eventManager[ROBOT_ANIM] = false;
	}
	// if(t >= 6.2f) {
	// 	eventManager[BKGND_MUSIC_PLAY] = true;
	// 	playerBkgnd->play();
	// }
	if(t >= 41.0f) {
		eventManager[DOOR_ANIM] = true;
	}
	if(t >= 25.7f && t <= 25.8f) {
		eventManager[ROBOT_ANIM] = true;
	}
	
	static const float DOOR_OPEN_SPEED = 0.075f;
	static const float ROBOT_MOVE_SPEED = 0.06f;
	static const float ROBOT_ANIM_SPEED = 0.99f;
	static const float ASTRO_ANIM_SPEED = 0.1f;
	static const float HOLOGRAM_UPDATE_SPEED = 0.5f;
	static const float CAMERA_SPEED = 0.025f;
	static const float CROSSFADE_SPEED = 0.3f;

	if(eventManager[CROSSFADE]) {
		crossT += CROSSFADE_SPEED * programglobal::deltaTime;
	}
	if(eventManager[DOOR_ANIM]) {
		doorT += DOOR_OPEN_SPEED * programglobal::deltaTime;
	}
	if(eventManager[ROBOT_ANIM]) {
		modelRobot->update(ROBOT_ANIM_SPEED * programglobal::deltaTime, 0);
		if(fmod(robotT, 0.04f) <= 0.001f) {
			playerRobotThump->play();
		}
		robotT += ROBOT_MOVE_SPEED * programglobal::deltaTime;
		robotT = std::min(robotT, 1.0f);
	}
	if(eventManager[CAMERA_MOVE]) {
		camera1->updateT(CAMERA_SPEED * programglobal::deltaTime);
	}
	
	hologramT += HOLOGRAM_UPDATE_SPEED * programglobal::deltaTime;
	modelAstro->update(ASTRO_ANIM_SPEED * programglobal::deltaTime, 0);
}

void labscene::uninit() {
	delete modelLab;
	delete modelDoor;
	delete modelMug;
	delete modelRobot;
	delete modelBLEND;
}

void labscene::keyboardfunc(int key) {
	if(programglobal::debugMode == CAMERA) {
		cameraRig->keyboardfunc(key);
	} else if(programglobal::debugMode == SPLINE) {
		robotSpline->keyboardfunc(key);
	}
	switch(key) {
	case XK_Tab:
		if(programglobal::debugMode == CAMERA) {
			cout<<cameraRig->getCamera()<<endl;
		}	
		if(programglobal::debugMode == SPLINE) {
			cout<<robotSpline->getSpline()<<endl;
		}
		cout<<"Current T = "<<t<<endl;
		break;
	}
}

camera* labscene::getCamera() {
	return camera1;
}