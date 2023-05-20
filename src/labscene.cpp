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
#include<CubeMapRenderTarget.h>
#include<glLight.h>
#include<assimp/postprocess.h>

using namespace std;
using namespace vmath;

#define CUBEMAP_SIZE 2048

static glmodel* modelLab;
static glmodel* modelDoor;
static glmodel* modelMug;
static glmodel* modelRobot;
static glmodel* modelAstro;

static BsplineInterpolator* bspRobot;

static CubeMapRenderTarget* envMapper;

static SceneLight* sceneLightManager;

static glshaderprogram* programStaticPBR;
static glshaderprogram* programDynamicPBR;
static glshaderprogram* programLight;
static glshaderprogram* programSkybox;
static glshaderprogram* programColor;

static GLuint skybox_vao,vbo;

static bool crt = true;

#ifdef DEBUG
static modelplacer* doorPlacer;
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
	try {
		programStaticPBR = new glshaderprogram({"shaders/pbr.vert", "shaders/pbrMain.frag"});
		programDynamicPBR = new glshaderprogram({"shaders/pbrDynamic.vert", "shaders/pbrMain.frag"});
		programLight = new glshaderprogram({"shaders/debug/lightSrc.vert", "shaders/debug/lightSrc.frag"});
		programSkybox = new glshaderprogram({"shaders/debug/rendercubemap.vert", "shaders/debug/rendercubemap.frag"});
		programColor = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});
	} catch(string errorString)  {
		throwErr(errorString);
	}
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
	modelLab = new glmodel("resources/models/spaceship/SpaceLab.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelDoor = new glmodel("resources/models/spaceship/door.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelMug = new glmodel("resources/models/mug/mug.glb", aiProcessPreset_TargetRealtime_Quality, true);
	modelRobot = new glmodel("resources/models/robot/robot.fbx", aiProcessPreset_TargetRealtime_Quality, true);
	modelAstro = new glmodel("resources/models/astronaut/MCAnim.glb", aiProcessPreset_TargetRealtime_Quality, true);
	
	bspRobot = new BsplineInterpolator(robotSpline);

	envMapper = new CubeMapRenderTarget(CUBEMAP_SIZE, CUBEMAP_SIZE, false);
	envMapper->setPosition(vec3(0.0f, 0.0f, 0.0f));
	
	sceneLightManager = new SceneLight(crt);
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
	splineRender = new SplineRenderer(bspRobot);
	splineRender->setRenderPoints(true);
	//Astronaut: vec3(-3.41f, -1.39f, 2.03f), vec3(0f, 0f, 0f), 0.00889994f
	doorPlacer = new modelplacer(vec3(-3.43f, -0.3f, 2.748f), vec3(0.0f, 0.0f, 0.0f), 1.0f);
#endif
}

static float t = 0.01f;
static float doort = 0.0f;

void labscene::render() {

	try {
	    if(crt){
            glBindFramebuffer(GL_FRAMEBUFFER, envMapper->FBO);
			glViewport(0, 0, envMapper->width, envMapper->height);

            for(int side = 0; side < 6; side++){
                //std::cout<<"i "<<side<<std::endl;
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
            crt = false;
        }

		programStaticPBR->use();
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("pMat"),1,GL_FALSE, programglobal::perspective);
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("vMat"),1,GL_FALSE, programglobal::currentCamera->matrix());
        glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"),1,GL_FALSE, translate(0.0f,0.0f,0.0f) * scale(1.0f,1.0f,1.0f));
        glUniform3fv(programStaticPBR->getUniformLocation("viewPos"),1, programglobal::currentCamera->position());
        // Lights data
        glUniform1i(programStaticPBR->getUniformLocation("specularGloss"),false);
        sceneLightManager->setLightUniform(programStaticPBR);
        modelLab->draw(programStaticPBR);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(mix(vec3(-3.3, -0.4f, 2.8f), vec3(-4.62f, -0.4f, 2.8f), doort)));
		modelDoor->draw(programStaticPBR);
		glUniformMatrix4fv(programStaticPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(-1.3f,-0.41f,-1.5f) * scale(0.08f,0.08f,0.08f));
		modelMug->draw(programStaticPBR);

        programDynamicPBR->use();
        glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"), 1,GL_FALSE, programglobal::perspective);
        glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"), 1,GL_FALSE, programglobal::currentCamera->matrix());
        vec3 position = bspRobot->interpolate(t - 0.01f);
		vec3 front = bspRobot->interpolate(t);
		glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"), 1, GL_FALSE, translate(position) * targetat(position, front, vec3(0.0f, 1.0f, 0.0f)) * scale(0.042f));
	    glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"), 1, programglobal::currentCamera->position());
        // Lights data
        glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"), true);
        sceneLightManager->setLightUniform(programDynamicPBR);
        modelRobot->update(0.005f, 0);
        modelRobot->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
        modelRobot->draw(programDynamicPBR,1); 

        programDynamicPBR->use();
        glUniformMatrix4fv(programDynamicPBR->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(programDynamicPBR->getUniformLocation("vMat"),1,GL_FALSE, programglobal::currentCamera->matrix());
        glUniformMatrix4fv(programDynamicPBR->getUniformLocation("mMat"),1,GL_FALSE, translate(-3.41f, -1.39f, 2.03f) * scale(1.0f));
        glUniform3fv(programDynamicPBR->getUniformLocation("viewPos"),1, programglobal::currentCamera->position());
        // Lights data
        glUniform1i(programDynamicPBR->getUniformLocation("specularGloss"),true);
        sceneLightManager->setLightUniform(programDynamicPBR);
        modelAstro->update(0.005f, 1);
        modelAstro->setBoneMatrixUniform(programDynamicPBR->getUniformLocation("bMat[0]"), 0);
        modelAstro->draw(programDynamicPBR,1);

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

#ifdef DEBUG
	splineRender->render(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f), selectedPoint, 0.01f);
#endif

	} catch(string errString) {
		throwErr(errString);
	}
	t += 0.0006f;
	t = std::min(t, 1.0f);
}

void labscene::uninit() {
	delete modelLab;
	delete modelDoor;
	delete modelMug;
	delete modelRobot;
}

void splineKeyboardFunc(int key) {
#ifdef DEBUG
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
#endif
}

void labscene::keyboardfunc(int key) {
#ifdef DEBUG
	doorPlacer->keyboardfunc(key);
	// astroPlacer->keyboardfunc(key);
	// splineKeyboardFunc(key);
	switch(key) {
	case XK_Tab:
		cout<<doorPlacer<<endl;
		// for(int i = 0; i < robotSpline.size(); i++) {
		// 	cout<<"\t"<<robotSpline[i]<<",\n";
		// }
		// cout<<"\b"<<endl;
		break;
	}
#endif
}