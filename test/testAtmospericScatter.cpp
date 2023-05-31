#include "camera.h"
#include <X11/X.h>
#include <assimp/postprocess.h>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <vmath.h>

#include <glshaderloader.h>
#include <gltextureloader.h>
#include <glmodelloader.h>
#include <vmath.h>
#include <errorlog.h>
#include <atmosphere.h>
#include <testAtmospericScatter.h>
#include <global.h>

using namespace std;
using namespace vmath;

static glmodel* sun;
static Atmosphere* atmosphere;
static glshaderprogram* color;
/*
mat4 projectionSky;

GLuint fbo,vao,vbo;

float SunRadius = 5.0f;
vec4 SunPos;
vec3 SunRotVec;
vec3 SunDir,SunWPos;
vec3 LightColor;
vec3 SunColor;
vec3 CamPos;

GLfloat AmbientIntensity;
GLfloat DiffuseIntensity;

float RefractionFactor;

GLint Samples = 4;
GLfloat Kr = 0.0030f;
GLfloat Km = 0.0015f;
GLfloat ESun = 16.0f;
GLfloat g = -0.98f;
GLfloat Exposure = 1.0f;
GLfloat InnerRadius = 10.0f * 100000.0f;
GLfloat OuterRadius = 10.25f * 100000.0f;
GLfloat Scale = 1.0f / (OuterRadius - InnerRadius);
GLfloat ScaleDepth = 0.25f;
GLfloat ScaleOverScaleDepth = Scale / ScaleDepth;

GLfloat WavelengthRed = 0.650f;
GLfloat WavelengthGreen = 0.570f;
GLfloat WavelengthBlue = 0.475f;

GLfloat WavelengthRed4 = powf(WavelengthRed, 4.0f);
GLfloat WavelengthGreen4 = powf(WavelengthGreen, 4.0f);
GLfloat WavelengthBlue4 = powf(WavelengthBlue,4.0f);

vec3 InvWaveLength = vec3(1.0f/WavelengthRed4, 1.0f/ WavelengthGreen4, 1.0f / WavelengthBlue4);
vec3 CameraPos;

void ComputeSkyDome(void);

GLfloat alpha;
GLboolean DecreaseAlpha;
GLboolean IncreaseAlpha;
GLboolean IsSunriseComplete;
GLboolean IsSunsetComplete;

bool m_bItNight;
bool m_bItDay;

glshaderprogram* atmosphere;

mat4 SetInfiniteProjectionMatrix(GLfloat Left, GLfloat Right, GLfloat Bottom, GLfloat Top, GLfloat Near, GLfloat Far);
vec4 mtransform(const mat4& m, const vec4& v){
    vec4 out;
    out[0] = m[0][0]*v[0] + m[1][0]*v[1] + m[2][0]*v[2] + m[3][0]*v[3];
    out[1] = m[0][1]*v[0] + m[1][1]*v[1] + m[2][1]*v[2] + m[3][1]*v[3];
	out[2] = m[0][2]*v[0] + m[1][2]*v[1] + m[2][2]*v[2] + m[3][2]*v[3];
	out[3] = m[0][3]*v[0] + m[1][3]*v[1] + m[2][3]*v[2] + m[3][3]*v[3];
	return out;
};
*/
void setupProgramAS(){
    try 
    {
        color = new glshaderprogram({"shaders/color.vert","shaders/color.frag"});
        /*
        atmosphere = new glshaderprogram({"shaders/AtmosphericScattering/Atmosphere.vert","shaders/AtmosphericScattering/Atmosphere.frag"});
        // initialize most of the uniforms
        atmosphere->use();
        glUniform3fv(atmosphere->getUniformLocation("v3InvWaveLength"), 1,InvWaveLength);
        //glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fCameraHeight"), InnerRadius);
        //glUniform1f(glGetUniformLocation(ShaderProgramObjectSky, "fCameraHeight2"), InnerRadius * InnerRadius);
        glUniform1f(atmosphere->getUniformLocation("fCameraHeight"),length(CameraPos));
        //glUniform1f(atmosphere->getUniformLocation("fCameraHeight2"), length(CameraPos) * length(CameraPos));
        glUniform1f(atmosphere->getUniformLocation("fInnerRadius"), InnerRadius);
        //glUniform1f(atmosphere->getUniformLocation("fInnerRadius2"), InnerRadius * InnerRadius);
        //glUniform1f(atmosphere->getUniformLocation( "fOuterRadius"), OuterRadius);
        //glUniform1f(atmosphere->getUniformLocation( "fOuterRadius2"), OuterRadius * OuterRadius);
        glUniform1f(atmosphere->getUniformLocation("fKrESun"), Kr * ESun);
        glUniform1f(atmosphere->getUniformLocation("fKmESun"), Km * ESun);
        glUniform1f(atmosphere->getUniformLocation("fKr4PI"), Kr * 4.0f * M_PI);
        glUniform1f(atmosphere->getUniformLocation("fKm4PI"), Km * 4.0f * M_PI);
        glUniform1f(atmosphere->getUniformLocation("fScale"), Scale);
        glUniform1f(atmosphere->getUniformLocation("fScaleDepth"), ScaleDepth);
        glUniform1f(atmosphere->getUniformLocation( "fScaleOverScaleDepth"), ScaleOverScaleDepth);
        glUniform1f(atmosphere->getUniformLocation( "g"), g);
        glUniform1f(atmosphere->getUniformLocation( "g2"), g * g);
        glUniform1i(atmosphere->getUniformLocation( "Samples"), Samples);
        glUniform1f(atmosphere->getUniformLocation( "alpha"), alpha);
        glUseProgram(0);
        */
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void initAS(){
    try {
        /*
        SunRotVec = normalize(vec3(0.0f,0.0f,-1.0f));
        //mat4 rot = (rotate(radians(-10.5f),SunRotVec) * vec4(-1200.0f,0.0,0.0f,1.0f));
        SunPos = mtransform(rotate(radians(-10.5f),SunRotVec), vec4(-1200.0f,0.0,0.0f,1.0f));
        SunDir = vec3(0.0f);
        SunWPos = vec3(0.0);

        //SunPos = vec3(-1.0f,0.0f,0.0f);

        RefractionFactor = 0.0f;

        LightColor = vec3(0.0f);
        AmbientIntensity = 0.0f;
        DiffuseIntensity = 0.0f;

        alpha = 1.0f;
        IncreaseAlpha =  GL_FALSE;
        DecreaseAlpha = GL_FALSE;
        IsSunriseComplete = GL_FALSE;
        IsSunsetComplete = GL_FALSE;

        m_bItNight = false;
        m_bItDay = true;    
        ComputeSkyDome();

        projectionSky = mat4::identity();

        projectionSky = SetInfiniteProjectionMatrix(-0.41421356237 * 1.778f, 0.41421356237 * 1.778f, -0.41421356237f, 0.41421356237f, 0.01f, 1000.0f);
        */
        sun = new glmodel("resources/models/sphere.glb",aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs,false);

        atmosphere = new Atmosphere(sun);

    } catch (string errorString) {
        throwErr(errorString);
    }
}

void renderAS(camera *cam,vec3 camPos){
    try {
        static float dt = 0.0f;
        
        // draw sphere

        color->use();
        glUniformMatrix4fv(color->getUniformLocation("mvpMatrix"),1,GL_FALSE,programglobal::perspective * cam->matrix() * translate(0.0f,0.0f,0.0f));
        glUniform4fv(color->getUniformLocation("color"),1,vec4(1.0f,0.0f,0.0f,1.0f));
        sun->draw(color,1,false);

        atmosphere->setProjView(programglobal::perspective, cam->matrix());
        atmosphere->setViewPos(camPos);
        atmosphere->render(dt);
        dt += 0.001f;
/*
        sky->use();
        glUniformMatrix4fv(sky->getUniformLocation("pMat"),1,GL_FALSE,programglobal::perspective);
        glUniformMatrix4fv(sky->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix()); 
        glUniformMatrix4fv(sky->getUniformLocation("mMat"),1,GL_FALSE,translate(0.0f,0.0f,0.0f) * scale(0.5f,0.5f,0.5f));
        glUniform3fv(sky->getUniformLocation("camPosition"),1,camPos);
        glUniform3fv(sky->getUniformLocation("lightPos"),1,vec3(0.0,0.0,-1.0));
        glUniform3fv(sky->getUniformLocation("C_R"),1,vec3(1.0,0.0,0.0));
        glUniform1f(sky->getUniformLocation("innerRadius"),10.0f);
        glUniform1f(sky->getUniformLocation("outerRadius"),10.25f);
        glUniform3fv(sky->getUniformLocation("mieTint"),1,vec3(1.0));
        glUniform1f(sky->getUniformLocation("SCALE_H_FACTOR"),6.0);
        //glUniform1f(sky->getUniformLocation("earthSizeCoefficient"),1.0);

        //glUniform3fv(sky->getUniformLocation("viewPos"),1,camPos);
        sun->draw(sky,1,false);


    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //Render Sky
    atmosphere->use();
    glUniformMatrix4fv(atmosphere->getUniformLocation("pMat"),1,GL_FALSE,projectionSky);
    glUniformMatrix4fv(atmosphere->getUniformLocation("vMat"),1,GL_FALSE,cam->matrix());
    glUniformMatrix4fv(atmosphere->getUniformLocation("mMat"),1,GL_FALSE,translate(camPos[0],camPos[1] - 10.0f * 100000.0f,camPos[2]));
    glUniform3fv(atmosphere->getUniformLocation("v3CameraPos"), 1, CameraPos);
    glUniform3fv(atmosphere->getUniformLocation("v3LightPos"), 1, SunDir);
    glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 112 * 3);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glUseProgram(0);


    // update stuff
    
    SunPos = mtransform(rotate(radians(-10.5f),SunRotVec), vec4(-1200.0f,0.0,0.0f,1.0f));
    

    SunDir = normalize(vec3(SunPos[0],SunPos[1],SunPos[2]));
    */
    } catch (string errorString) {
        throwErr(errorString);
    }
}

void uninitAS(){

/*
    if(sky)
        delete sky;
    if(SunDepthTest)
        delete SunDepthTest;
    if(BlurH)
        delete BlurH;
    if(BlurV)
        delete BlurV;
    if(SunRaysLenseFlareHalo)
        delete SunRaysLenseFlareHalo;
*/
    if(atmosphere)
        delete atmosphere;
    if(sun)
        delete sun;

    if(color)
        delete color;
}
/*
void ComputeSkyDome(void){

	vec3 *SkyDomeVertices = new vec3[112 * 3], va, vb, vc, vd;

	float stepa = (float)M_PI * 2.0f / 16, startb = asin(InnerRadius / OuterRadius), stepb = ((float)M_PI_2 - startb) / 4;
	int pos = 0;

	for(int y = 0; y < 3; y++)
	{
		float b = startb + stepb * y;

		for(int x = 0; x < 16; x++)
		{
			float a = stepa * x;

			va = vec3(sin(a) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a) * cos(b) * OuterRadius);
			vb = vec3(sin(a + stepa) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a + stepa) * cos(b) * OuterRadius);
			vc = vec3(sin(a + stepa) * cos(b + stepb) * OuterRadius, sin(b + stepb) * OuterRadius, -cos(a + stepa) * cos(b + stepb) * OuterRadius);
			vd = vec3(sin(a) * cos(b + stepb) * OuterRadius, sin(b + stepb) * OuterRadius, -cos(a) * cos(b + stepb) * OuterRadius);

			SkyDomeVertices[pos + 0] = va;
			SkyDomeVertices[pos + 1] = vb;
			SkyDomeVertices[pos + 2] = vc;

			pos += 3;

			SkyDomeVertices[pos + 0] = vc;
			SkyDomeVertices[pos + 1] = vd;
			SkyDomeVertices[pos + 2] = va;

			pos += 3;
		}
	}

	float b = startb + stepb * 3;

	for(int x = 0; x < 16; x++)
	{
		float a = stepa * x;

		va = vec3(sin(a) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a) * cos(b) * OuterRadius);
		vb = vec3(sin(a + stepa) * cos(b) * OuterRadius, sin(b) * OuterRadius, -cos(a + stepa) * cos(b) * OuterRadius);
		vc = vec3(0, OuterRadius, 0);

		SkyDomeVertices[pos + 0] = va;
		SkyDomeVertices[pos + 1] = vb;
		SkyDomeVertices[pos + 2] = vc;

		pos += 3;
	}
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Vbo_Model_Vertex
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 112 * 3 * 3 * sizeof(float), SkyDomeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	delete[] SkyDomeVertices;
}

mat4 SetInfiniteProjectionMatrix(GLfloat Left, GLfloat Right, GLfloat Bottom, GLfloat Top, GLfloat Near, GLfloat Far){
    mat4 result = mat4::identity();

	if ((Right == Left) || (Top == Bottom) || (Near == Far) || (Near < 0.0) || (Far < 0.0))
		return result;

	result[0][0] = (2.0f * Near) / (Right - Left);
	result[1][1] = (2.0f * Near) / (Top - Bottom);

	result[2][0] = (Right + Left) / (Right - Left);
	result[2][1] = (Top + Bottom) / (Top - Bottom);
	//result[2][2] = -(Far + Near) / (Far - Near);
	result[2][2] = -1.0f;
	result[2][3] = -1.0f;

	//result[3][2] = -(2.0f * Far * Near) / (Far - Near);
	result[3][2] = -(2.0f * Near);
	result[3][3] = 0.0f;
    return result;
}*/