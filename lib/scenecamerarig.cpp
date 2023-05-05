#include"../include/scenecamerarig.h"
#include<X11/keysym.h>

using namespace std;
using namespace vmath;

#define PATH_LINE_COLOR vec4(0.0f, 0.0f, 0.0f, 1.0f)
#define FRONT_LINE_COLOR vec4(1.0f, 1.0f, 1.0f, 1.0f)
#define PATH_POINT_COLOR vec4(0.0f, 1.0f, 0.0f, 1.0f)
#define SELECTED_PATH_POINT_COLOR vec4(1.0f, 1.0f, 0.0f, 1.0f)
#define FRONT_POINT_COLOR vec4(0.0f, 0.0f, 1.0f, 0.0f)
#define SELECTED_FRONT_POINT_COLOR vec4(0.0f, 1.0f, 1.0f, 1.0f)

/*********************************************************************/
/*                              SceneCameraRig                       */
/*********************************************************************/
sceneCameraRig::sceneCameraRig(std::vector<vmath::vec3> positionKeyFrames, std::vector<vmath::vec3> frontKeyFrames)
    : isRenderPath(true),
      isRenderFront(false),
      isRenderPathToFront(false),
      scalingFactor(1.0f),
	  selectedPathPoint(0),
	  selectedFrontPoint(0)
{
    program = new glshaderprogram({"shaders/cameraRig.vert", "shaders/cameraRig.frag"});

    glCreateVertexArrays(1, &vaoPoint);
    glCreateBuffers(1, &vboPoint);
    glCreateVertexArrays(1, &vaoPathToFront);
    glCreateBuffers(1, &vboPathToFront);

	this->positionKeyFrames = positionKeyFrames;
	this->frontKeyFrames = frontKeyFrames;

    mountCamera = new sceneCamera(positionKeyFrames, frontKeyFrames);
    pathRenderer = new SplineRenderer(mountCamera->m_bspPositions);
    frontRenderer = new SplineRenderer(mountCamera->m_bspFront);

    loadGeometry();
}

sceneCameraRig::~sceneCameraRig()
{
    if (frontRenderer)
    {
        delete frontRenderer;
        frontRenderer = NULL;
    }
    if (pathRenderer)
    {
        delete pathRenderer;
        pathRenderer = NULL;
    }
    if (vboPathToFront)
    {
        glDeleteBuffers(1, &vboPathToFront);
        vboPathToFront = 0;
    }
    if (vaoPathToFront)
    {
        glDeleteVertexArrays(1, &vaoPathToFront);
        vaoPathToFront = 0;
    }
    if (vboPoint)
    {
        glDeleteBuffers(1, &vboPoint);
        vboPoint = 0;
    }
    if (vaoPoint)
    {
        glDeleteVertexArrays(1, &vaoPoint);
        vaoPoint = 0;
    }
    if(program)
    {
        delete program;
        program = NULL;
    }
}

void sceneCameraRig::loadGeometry(void)
{
    /* render a point as a cube */
    const float verts[] = {
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,

        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,

        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f};

    glBindVertexArray(vaoPoint);
    glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    /* render path-to-front vector as a line (end-points updated dynamically) */
    glBindVertexArray(vaoPathToFront);
    glBindBuffer(GL_ARRAY_BUFFER, vboPathToFront);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 2, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
}

void sceneCameraRig::render() const
{
    if (isRenderPath)
        pathRenderer->render(PATH_LINE_COLOR, PATH_POINT_COLOR, SELECTED_PATH_POINT_COLOR, selectedPathPoint, this->scalingFactor);
    if (isRenderFront)
        frontRenderer->render(FRONT_LINE_COLOR, FRONT_POINT_COLOR, SELECTED_FRONT_POINT_COLOR, selectedFrontPoint, this->scalingFactor);

    if (isRenderPathToFront)
    {
        // get current points on path and front spline
        vec3 point_front[2] = {
            mountCamera->m_bspPositions->interpolate(mountCamera->t),
            mountCamera->m_bspFront->interpolate(mountCamera->t)
		};

        program->use();
        glUniformMatrix4fv(1, 1, GL_FALSE, programglobal::currentCamera->matrix());
        glUniformMatrix4fv(2, 1, GL_FALSE, programglobal::perspective);

        // draw a line connecting the 2 points
        glUniform1i(3, 0); // isPoint = false
        glBindVertexArray(vaoPathToFront);
        glBindBuffer(GL_ARRAY_BUFFER, vboPathToFront);
        glBufferData(GL_ARRAY_BUFFER, sizeof(point_front), point_front, GL_DYNAMIC_DRAW);
        glUniformMatrix4fv(0, 1, GL_FALSE, mat4::identity());
        glDrawArrays(GL_LINES, 0, 2);

        // draw the 2 points
        glUniform1i(3, 1); // isPoint = true
        glBindVertexArray(vaoPoint);
        glUniformMatrix4fv(0, 1, GL_FALSE, translate(point_front[0]) * scale(this->scalingFactor));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glUniformMatrix4fv(0, 1, GL_FALSE, translate(point_front[1]) * scale(this->scalingFactor));
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void sceneCameraRig::updateT(float speed)
{
    mountCamera->t = std::min(mountCamera->t + speed, 1.0f);
}

void sceneCameraRig::resetT(void) {
	mountCamera->t = 0.0f;
}

void sceneCameraRig::setRenderPath(bool setting)
{
    isRenderPath = setting;
}

void sceneCameraRig::setRenderPathPoints(bool setting)
{	this->isRenderPathPoints;
    pathRenderer->setRenderPoints(setting);
}

void sceneCameraRig::setRenderFront(bool setting)
{
    isRenderFront = setting;
}

void sceneCameraRig::setRenderFrontPoints(bool setting)
{
	this->isRenderFrontPoints;
    frontRenderer->setRenderPoints(setting);
}

void sceneCameraRig::setRenderPathToFront(bool setting)
{
    isRenderPathToFront = setting;
}

void sceneCameraRig::setScalingFactor(float scalingFactor) {
	this->scalingFactor = scalingFactor;
}

sceneCamera* sceneCameraRig::getCamera() {
	return this->mountCamera;
}

void sceneCameraRig::keyboardfunc(int key) {
	bool refreshPos = false;
	bool refreshFront = false;
	switch(key) {
	//Front Point Select
	case XK_v:
		selectedFrontPoint = (selectedFrontPoint == 0) ? frontKeyFrames.size() - 1 : (selectedFrontPoint - 1) % this->frontKeyFrames.size();
		break;
	case XK_b:
		selectedFrontPoint = (selectedFrontPoint + 1) % this->frontKeyFrames.size();
		break;
	//Path Point Select
	case XK_n:
		selectedPathPoint = (selectedPathPoint == 0) ? positionKeyFrames.size() - 1 : (selectedPathPoint - 1) % this->positionKeyFrames.size();
		break;
	case XK_m:
		selectedPathPoint = (selectedPathPoint + 1) % this->positionKeyFrames.size();
		break;
	//Front Point Move Z
	case XK_t:
		this->frontKeyFrames[selectedFrontPoint][2] += 1.0f;
		refreshFront = true;
		break;
	case XK_g:
		this->frontKeyFrames[selectedFrontPoint][2] -= 1.0f;
		refreshFront = true;
		break;
	//Front Point Move X
	case XK_h:
		this->frontKeyFrames[selectedFrontPoint][0] += 1.0f;
		refreshFront = true;
		break;
	case XK_f:
		this->frontKeyFrames[selectedFrontPoint][0] -= 1.0f;
		refreshFront = true;
		break;
	//Front Point Move Y
	case XK_y:
		this->frontKeyFrames[selectedFrontPoint][1] += 1.0f;
		refreshFront = true;
		break;
	case XK_r:
		this->frontKeyFrames[selectedFrontPoint][1] -= 1.0f;
		refreshFront = true;
		break;
	//Path Point Move Z
	case XK_i:
		this->positionKeyFrames[selectedPathPoint][2] += 1.0f;
		refreshPos = true;
		break;
	case XK_k:
		this->positionKeyFrames[selectedPathPoint][2] -= 1.0f;
		refreshPos = true;
		break;
	//Path Point Move X
	case XK_l:
		this->positionKeyFrames[selectedPathPoint][0] += 1.0f;
		refreshPos = true;
		break;
	case XK_j:
		this->positionKeyFrames[selectedPathPoint][0] -= 1.0f;
		refreshPos = true;
		break;
	//Path Point Move Y
	case XK_o:
		this->positionKeyFrames[selectedPathPoint][1] += 1.0f;
		refreshPos = true;
		break;
	case XK_u:
		this->positionKeyFrames[selectedPathPoint][1] -= 1.0f;
		refreshPos = true;
		break;
	}

	if(refreshPos) {
		delete this->mountCamera->m_bspPositions;
		this->mountCamera->m_bspPositions = new BsplineInterpolator(this->positionKeyFrames);
		delete this->pathRenderer;
		this->pathRenderer = new SplineRenderer(this->mountCamera->m_bspPositions);
		this->pathRenderer->setRenderPoints(this->isRenderPathPoints);
	}
	if(refreshFront) {
		delete this->mountCamera->m_bspFront;
		this->mountCamera->m_bspFront = new BsplineInterpolator(this->frontKeyFrames);
		delete this->frontRenderer;
		this->frontRenderer = new SplineRenderer(this->mountCamera->m_bspFront);
		this->frontRenderer->setRenderPoints(this->isRenderFrontPoints);
	}
}

/********************************** EOF ******************************/
