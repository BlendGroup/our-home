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
#include<godrays.h>
#include<crossfade.h>
#include<eventmanager.h>

using namespace std;
using namespace vmath;

#define CUBEMAP_SIZE 2048

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
static glshaderprogram* programMugSteam;

static GLuint skybox_vao,vbo;
static GLuint emptyvao;

static audioplayer *playerBkgnd;
static audioplayer *playerRobotThump;
static audioplayer *playerOpeningDoor;
static audioplayer *playerHologramBeeps;

static godrays* godraysDoor;

#ifdef DEBUG
static modelplacer* doorPlacer;
static sceneCameraRig* cameraRig;
static SplineAdjuster* robotSpline;
#endif

enum tvalues {
	ROBOT_T,
	SFX_ROBOT_THUMP_T,
	ASTRONAUT_T,
	CAMERA_T,
	DOOR_T,
	HOLOGRAM_T,
	CROSSIN_T,
	CROSSOUT_T
};

static eventmanager* labevents;
static GLfloat hologramT	= 0.0f;
static GLfloat steamT		= 0.0f;

extern GLuint texTitleSceneFinal;
GLuint texLabSceneFinal;

void labscene::setupProgram() {
	try {
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
		programLight = new glshaderprogram({"shaders/debug/lightSrc.vert", "shaders/debug/lightSrc.frag"});
		programSkybox = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/debug/rendercubemap.frag"});
		programColor = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
		programHologram = new glshaderprogram({"shaders/hologram/hologram.vert","shaders/hologram/hologram.frag"});
		programMugSteam = new glshaderprogram({{"shaders/steam/steam.vert"}, {"shaders/steam/steam.frag"}});
	} catch(string errorString)  {
		throwErr(errorString);
	}
}

void labscene::setupCamera() {
	vector<vec3> positionKeyFrames = {
		vec3(-1.96f, -0.27f, -1.13f),
		vec3(-1.92f, -0.29f, -1.11f),
		vec3(-1.89f, -0.27f, -1.36f),
		vec3(-1.67f, -0.3f, -1.64f),
		vec3(-1.27f, -0.18f, -1.79f),
		vec3(-0.82f, 0.28f, -1.85f),
		vec3(-1.17f, 0.06f, -1.1f),
		vec3(-2.9f, -0.2f, -0.51f),
		vec3(-3.49f, -0.41f, 0.05f)
	};
    
	vector<vec3> frontKeyFrames = {
		vec3(-1.96f, -0.27f, -1.65f),
		vec3(-1.74f, -0.3f, -1.53f),
		vec3(-1.3f, -0.3f, -1.38f),
		vec3(-0.36f, -0.38f, -1.23f),
		vec3(0.09f, -0.28f, -0.98f),
		vec3(-0.21f, -0.26f, -0.37f),
		vec3(-1.67f, -0.42f, 0.45f),
		vec3(-3.19f, -0.36f, 0.96f),
		vec3(-3.49f, -0.27f, 2.02f)
	};

	camera1 = new sceneCamera(positionKeyFrames, frontKeyFrames);

#ifdef DEBUG
	cameraRig = new sceneCameraRig(camera1);
	cameraRig->setRenderFront(true);
	cameraRig->setRenderFrontPoints(true);
	cameraRig->setRenderPath(true);
	cameraRig->setRenderPathPoints(true);
	cameraRig->setRenderPathToFront(true);
	cameraRig->setScalingFactor(0.01f);
#endif
}

void labscene::init() {
	labevents = new eventmanager({
		{CROSSIN_T, { 0.0f, 3.36f }},
		{CAMERA_T, { 3.36f, 40.0f }},
		{ROBOT_T, { 20.5f, 19.5f }},
		{SFX_ROBOT_THUMP_T, { 20.5f, 16.5f }},
		{DOOR_T, { 41.0f, 9.0f }},
		{CROSSOUT_T, { 48.2f, 4.0f }},
	});

	playerBkgnd = new audioplayer("resources/audio/TheLegendOfKai.wav");
	playerRobotThump = new audioplayer("resources/audio/MetallicThumps.wav");
	playerOpeningDoor = new audioplayer("resources/audio/OpeningDoor.wav");
	playerHologramBeeps = new audioplayer("resources/audio/HologramAndBeeps.wav");

	modelLab = new glmodel("resources/models/spaceship/SpaceLab.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelDoor = new glmodel("resources/models/spaceship/door.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelMug = new glmodel("resources/models/mug/mug.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelRobot = new glmodel("resources/models/robot/robot.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelAstro = new glmodel("resources/models/astronaut/MCAnim.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelBLEND = new glmodel("resources/models/blendlogo/BLEND.glb",aiProcessPreset_TargetRealtime_Quality,false);

	godraysDoor = new godrays();
	godraysDoor->setDecay(0.98f);
	godraysDoor->setDensity(1.2f);
	godraysDoor->setExposure(1.0f);
	godraysDoor->setSamples(125);
	godraysDoor->setWeight(0.06f);

	bspRobot = new BsplineInterpolator({
		vec3(2.14f, -1.067f, 1.7f),
		vec3(1.83f, -1.067f, 0.79f),
		vec3(0.2f, -1.067f, 0.72f),
		vec3(-0.9f, -1.067f, 0.82f),
		vec3(-2.07f, -1.067f, 1.05f),
		vec3(-2.73f, -1.067f, 1.72f)
	});
#ifdef DEBUG
	robotSpline = new SplineAdjuster(bspRobot);
	robotSpline->setRenderPath(true);
	robotSpline->setRenderPoints(true);
	robotSpline->setScalingFactor(0.01f);
	
	doorPlacer = new modelplacer(vec3(-3.41f, -1.39f, 2.03f), vec3(0.0f, 0.0f, 0.0f), 1.0f);
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

	glGenVertexArrays(1, &emptyvao);
	
	unsigned char white[] = {255, 255, 255, 255};
	glGenTextures(1, &texLabSceneFinal);
	glBindTexture(GL_TEXTURE_2D, texLabSceneFinal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
	glBindTexture(GL_TEXTURE_2D, 0);

	programStaticPBR = new glshaderprogram({"shaders/pbrStatic.vert", "shaders/pbrMain.frag"});
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
		camera1->setT((*labevents)[CAMERA_T]);

		programStaticPBR->use();
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"),1,GL_FALSE, programglobal::perspective);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"),1,GL_FALSE, programglobal::currentCamera->matrix());
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"),1,GL_FALSE, translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
		glUniform3fv(programStaticPBR->getUniformLocation("viewPos"),1, programglobal::currentCamera->position());
		// Lights data
		glUniform1i(programStaticPBR->getUniformLocation("specularGloss"),false);
		sceneLightManager->setLightUniform(programStaticPBR);
		modelLab->draw(programStaticPBR);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(mix(vec3(-3.3, -0.4f, 2.8f), vec3(-4.62f, -0.4f, 2.8f), (*labevents)[DOOR_T])));
		modelDoor->draw(programStaticPBR);
		static const mat4 mugTransform = translate(-1.20599f, -0.41f, -1.363f);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, mugTransform * scale(0.08f));
		modelMug->draw(programStaticPBR);

		programDynamicPBR->use();
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"), 1,GL_FALSE, programglobal::perspective);
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"), 1,GL_FALSE, programglobal::currentCamera->matrix());
		vec3 position = bspRobot->interpolate((*labevents)[ROBOT_T] - 0.01f);
		vec3 front = bspRobot->interpolate((*labevents)[ROBOT_T]);
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
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"),1,GL_FALSE, translate(-3.41f, -1.39f, 2.03f) * scale(0.86f));
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
        glUniformMatrix4fv(programHologram->getUniformLocation("mMat"),1,GL_FALSE, translate(-1.96f, -0.27f, -1.68f) * scale(0.11f));
		glUniform4fv(programHologram->getUniformLocation("MainColor"),1,vec4(0.0f,0.0f,1.0f,1.0f));
		glUniform4fv(programHologram->getUniformLocation("RimColor"),1,vec4(0.0f,1.0f,1.0f,1.0f));
		glUniform1f(programHologram->getUniformLocation("gTime"), hologramT);
		glUniform1f(programHologram->getUniformLocation("GlitchIntensity"), 5.0f);
		glUniform1f(programHologram->getUniformLocation("GlitchSpeed"), 100.0f);
		glUniform1f(programHologram->getUniformLocation("BarSpeed"),2.0f);
		glUniform1f(programHologram->getUniformLocation("BarDistance"),100.0f);
		glUniform1f(programHologram->getUniformLocation("alpha"), 1.0f);
		glUniform1f(programHologram->getUniformLocation("FlickerSpeed"),10.0f);
		glUniform1f(programHologram->getUniformLocation("RimPower"),5.0f);
		glUniform1f(programHologram->getUniformLocation("EmissionPower"), (*labevents)[CROSSIN_T]);
		// glUniform1f(programHologram->getUniformLocation("GlowSpeed"),1.0f);
		// glUniform1f(programHologram->getUniformLocation("GlowDistance"),1.0f);
		modelBLEND->draw(programHologram,1,false);
		glDepthMask(GL_TRUE);

		// a cheap cylindrical billboard for mug steam quad
		programMugSteam->use();
		mat4 mugSteamQuadMVTransform = programglobal::currentCamera->matrix() * translate(-0.04f, 0.15f, 0.0f) * mugTransform;
		mugSteamQuadMVTransform[0][0] = 1.0f;
		mugSteamQuadMVTransform[0][1] = 0.0f;
		mugSteamQuadMVTransform[0][2] = 0.0f;
		mugSteamQuadMVTransform[2][0] = 0.0f;
		mugSteamQuadMVTransform[2][1] = 0.0f;
		mugSteamQuadMVTransform[2][2] = 1.0f;
		glUniformMatrix4fv(programMugSteam->getUniformLocation("mvpMatrix"), 1, GL_FALSE, programglobal::perspective * mugSteamQuadMVTransform * scale(0.07f));
		glUniform1f(programMugSteam->getUniformLocation("time"), steamT);
		programglobal::shapeRenderer->renderQuad();
		
		glDisable(GL_BLEND);
		
		programColor->use();
		glUniformMatrix4fv(programColor->getUniformLocation("mvpMatrix"), 1, GL_FALSE, programglobal::perspective * programglobal::currentCamera->matrix() * translate(-3.45f, -0.3f, 2.828f));
		glUniform4f(programColor->getUniformLocation("color"), 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform4fv(programColor->getUniformLocation("emissive"), 1, vec4(0.0f, 0.0f, 0.0f, 0.0f));
		glUniform4fv(programColor->getUniformLocation("occlusion"), 1, vec4(1.0f, 1.0f, 1.0f, 1.0f));
		programglobal::shapeRenderer->renderQuad();
		godraysDoor->setScreenSpaceCoords(programglobal::perspective * programglobal::currentCamera->matrix() * translate(-3.45f, -0.3f, 2.828f), vec4(0.0f, 0.0f, 0.0f, 1.0f));

		// render light src
		// programLight->use();
		// glUniformMatrix4fv(programLight->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
		// glUniformMatrix4fv(programLight->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix()); 
		// sceneLightManager->renderSceneLights(programLight);

		// render to cubemap test
		// programSkybox->use();
		// glUniformMatrix4fv(programSkybox->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
		// glUniformMatrix4fv(programSkybox->getUniformLocation("vMat"),1,GL_FALSE,programglobal::currentCamera->matrix());
		// glBindVertexArray(skybox_vao);
		// glBindTextureUnit(0,envMapper->cubemap_texture);
		// glDrawArrays(GL_TRIANGLES, 0, 36);

		if((*labevents)[CROSSIN_T] < 1.0f) {
			crossfader::render(texTitleSceneFinal, (*labevents)[CROSSIN_T]);
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
	labevents->resetT();
}

void labscene::update() {
	labevents->increment();
	static const float ROBOT_ANIM_SPEED = 0.99f;
	static const float ASTRO_ANIM_SPEED = 0.1f;
	static const float HOLOGRAM_UPDATE_SPEED = 0.5f;
	static const float STEAM_UPDATE_SPEED = 0.5f;
	
	if(labevents->getT() >= 32.0f) {
		playerBkgnd->play();
	}
	if(labevents->getT() > 41.0f && labevents->getT() < 54.0f) {
		playerOpeningDoor->play();
	} else {
		playerOpeningDoor->pause();
	}
	if(labevents->getT() > 0.00001f && labevents->getT() < 25.0f) {
		playerHologramBeeps->play();
	} else {
		playerHologramBeeps->pause();
	}

	if((*labevents)[ROBOT_T] >= 0.00001f && (*labevents)[ROBOT_T] <= 0.99999f) {
		modelRobot->update(ROBOT_ANIM_SPEED * programglobal::deltaTime, 0);
		playerRobotThump->play();
	} else {
		playerRobotThump->pause();
	}
	hologramT += HOLOGRAM_UPDATE_SPEED * programglobal::deltaTime;
	steamT += STEAM_UPDATE_SPEED * programglobal::deltaTime;
	modelAstro->update(ASTRO_ANIM_SPEED * programglobal::deltaTime, 0);

	if((*labevents)[CROSSOUT_T] >= 1.0f) {
		cout<<"Lab Scene Duration: "<<labevents->getT()<<endl;
		playNextScene();
	}
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
	} else if(programglobal::debugMode == MODEL) {
		doorPlacer->keyboardfunc(key);
	} else if(programglobal::debugMode == LIGHT){
	}
	switch(key) {
	case XK_Up:
		(*labevents) += 0.4f;
		break;
	case XK_Down:
		(*labevents) -= 0.4f;
		break;
	case XK_Tab:
		if(programglobal::debugMode == CAMERA) {
			cout<<cameraRig->getCamera()<<endl;
		}	
		if(programglobal::debugMode == SPLINE) {
			cout<<robotSpline->getSpline()<<endl;
		}
		if(programglobal::debugMode == MODEL) {
			cout<<doorPlacer<<endl;
		}
		break;
	}
}

camera* labscene::getCamera() {
	return camera1;
}

void labscene::crossfade() {
	if((*labevents)[DOOR_T] > 0.0f) {
		godraysDoor->renderRays();
	}
	if((*labevents)[CROSSOUT_T] > 0.0f) {
		crossfader::render(texLabSceneFinal, 1.0f - (*labevents)[CROSSOUT_T]);
	}
}