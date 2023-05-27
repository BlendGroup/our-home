#include<scenecamerarig.h>
#include<X11/keysym.h>

using namespace std;
using namespace vmath;

#define PATH_LINE_COLOR vec4(0.0f, 0.0f, 0.0f, 1.0f)
#define FRONT_LINE_COLOR vec4(1.0f, 1.0f, 1.0f, 1.0f)
#define PATH_POINT_COLOR vec4(0.0f, 1.0f, 0.0f, 1.0f)
#define SELECTED_PATH_POINT_COLOR vec4(1.0f, 1.0f, 0.0f, 1.0f)
#define FRONT_POINT_COLOR vec4(0.0f, 0.0f, 1.0f, 0.0f)
#define SELECTED_FRONT_POINT_COLOR vec4(0.0f, 1.0f, 1.0f, 1.0f)
#define CAMERA_RIG_COLOR vec4(1.0f, 0.0f, 0.0f, 1.0f)

/*********************************************************************/
/*                              SceneCameraRig                       */
/*********************************************************************/
sceneCameraRig::sceneCameraRig(sceneCamera* scenecam)
    : isRenderPath(false),
      isRenderFront(false),
      isRenderPathToFront(false),
	  isRenderPathPoints(false),
	  isRenderFrontPoints(false),
      scalingFactor(1.0f),
	  selectedPathPoint(0),
	  selectedFrontPoint(0)
{
	if(scenecam) {
		program = new glshaderprogram({"shaders/color.vert", "shaders/color.frag"});

		glCreateVertexArrays(1, &vaoPoint);
		glCreateBuffers(1, &vboPoint);
		glCreateVertexArrays(1, &vaoPathToFront);
		glCreateBuffers(1, &vboPathToFront);

		mountCamera = scenecam;
		pathRenderer = new SplineRenderer(mountCamera->m_bspPositions);
		frontRenderer = new SplineRenderer(mountCamera->m_bspFront);

		loadGeometry();
	} else {
		throwErr("NULL pointer passed to constructor");
	}
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
		glUniform4fv(program->getUniformLocation("color"), 1, CAMERA_RIG_COLOR);
		glUniform4fv(program->getUniformLocation("emissive"), 1, vec4(0.0f, 0.0f, 0.0f, 0.0f));
		glUniform4fv(program->getUniformLocation("occlusion"), 1, vec4(0.0f, 0.0f, 0.0f, 0.0f));
		mat4 vpMat = programglobal::perspective * programglobal::currentCamera->matrix();

        // draw a line connecting the 2 points
        glBindVertexArray(vaoPathToFront);
        glBindBuffer(GL_ARRAY_BUFFER, vboPathToFront);
        glBufferData(GL_ARRAY_BUFFER, sizeof(point_front), point_front, GL_DYNAMIC_DRAW);
        glUniformMatrix4fv(program->getUniformLocation("mvpMatrix"), 1, GL_FALSE, vpMat);
        glDrawArrays(GL_LINES, 0, 2);

        // draw the 2 points
        glBindVertexArray(vaoPoint);
        glUniformMatrix4fv(program->getUniformLocation("mvpMatrix"), 1, GL_FALSE, vpMat * translate(point_front[0]) * scale(this->scalingFactor));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glUniformMatrix4fv(program->getUniformLocation("mvpMatrix"), 1, GL_FALSE, vpMat * translate(point_front[1]) * scale(this->scalingFactor));
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
{	this->isRenderPathPoints = setting;
    pathRenderer->setRenderPoints(setting);
}

void sceneCameraRig::setRenderFront(bool setting)
{
    isRenderFront = setting;
}

void sceneCameraRig::setRenderFrontPoints(bool setting)
{
	this->isRenderFrontPoints = setting;
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
	case XK_Left:
		mountCamera->updateT(-0.01f);
		break;
	case XK_Right:
		mountCamera->updateT(0.01f);
		break;
	//Front Point Select
	case XK_v:
		selectedFrontPoint = (selectedFrontPoint == 0) ? mountCamera->m_bspFront->m_pointsVec.size() - 1 : (selectedFrontPoint - 1) % this->mountCamera->m_bspFront->m_pointsVec.size();
		break;
	case XK_b:
		selectedFrontPoint = (selectedFrontPoint + 1) % this->mountCamera->m_bspFront->m_pointsVec.size();
		break;
	//Path Point Select
	case XK_n:
		selectedPathPoint = (selectedPathPoint == 0) ? mountCamera->m_bspPositions->m_pointsVec.size() - 1 : (selectedPathPoint - 1) % this->mountCamera->m_bspPositions->m_pointsVec.size();
		break;
	case XK_m:
		selectedPathPoint = (selectedPathPoint + 1) % this->mountCamera->m_bspPositions->m_pointsVec.size();
		break;
	//Front Point Move Z
	case XK_t:
		this->mountCamera->m_bspFront->m_pointsVec[selectedFrontPoint][2] += CAMERA_RIG_SCALER;
		refreshFront = true;
		break;
	case XK_g:
		this->mountCamera->m_bspFront->m_pointsVec[selectedFrontPoint][2] -= CAMERA_RIG_SCALER;
		refreshFront = true;
		break;
	//Front Point Move X
	case XK_h:
		this->mountCamera->m_bspFront->m_pointsVec[selectedFrontPoint][0] += CAMERA_RIG_SCALER;
		refreshFront = true;
		break;
	case XK_f:
		this->mountCamera->m_bspFront->m_pointsVec[selectedFrontPoint][0] -= CAMERA_RIG_SCALER;
		refreshFront = true;
		break;
	//Front Point Move Y
	case XK_y:
		this->mountCamera->m_bspFront->m_pointsVec[selectedFrontPoint][1] += CAMERA_RIG_SCALER;
		refreshFront = true;
		break;
	case XK_r:
		this->mountCamera->m_bspFront->m_pointsVec[selectedFrontPoint][1] -= CAMERA_RIG_SCALER;
		refreshFront = true;
		break;
	//Path Point Move Z
	case XK_i:
		this->mountCamera->m_bspPositions->m_pointsVec[selectedPathPoint][2] += CAMERA_RIG_SCALER;
		refreshPos = true;
		break;
	case XK_k:
		this->mountCamera->m_bspPositions->m_pointsVec[selectedPathPoint][2] -= CAMERA_RIG_SCALER;
		refreshPos = true;
		break;
	//Path Point Move X
	case XK_l:
		this->mountCamera->m_bspPositions->m_pointsVec[selectedPathPoint][0] += CAMERA_RIG_SCALER;
		refreshPos = true;
		break;
	case XK_j:
		this->mountCamera->m_bspPositions->m_pointsVec[selectedPathPoint][0] -= CAMERA_RIG_SCALER;
		refreshPos = true;
		break;
	//Path Point Move Y
	case XK_o:
		this->mountCamera->m_bspPositions->m_pointsVec[selectedPathPoint][1] += CAMERA_RIG_SCALER;
		refreshPos = true;
		break;
	case XK_u:
		this->mountCamera->m_bspPositions->m_pointsVec[selectedPathPoint][1] -= CAMERA_RIG_SCALER;
		refreshPos = true;
		break;
	//Add/Remove Path Points
	case XK_bracketleft:
		this->mountCamera->m_bspPositions->m_pointsVec.insert(this->mountCamera->m_bspPositions->m_pointsVec.begin() + selectedPathPoint + 1, vec3(0.0f, 0.0f, 0.0f));
		refreshPos = true;
		break;
	case XK_bracketright:
		this->mountCamera->m_bspPositions->m_pointsVec.erase(this->mountCamera->m_bspPositions->m_pointsVec.begin() + selectedPathPoint);
		selectedPathPoint = selectedPathPoint % this->mountCamera->m_bspPositions->m_pointsVec.size();
		refreshPos = true;
		break;
	//Add/Remove Front Points
	case XK_comma:
		this->mountCamera->m_bspFront->m_pointsVec.insert(this->mountCamera->m_bspFront->m_pointsVec.begin() + selectedFrontPoint + 1, vec3(0.0f, 0.0f, 0.0f));
		refreshFront = true;
		break;
	case XK_period:
		this->mountCamera->m_bspFront->m_pointsVec.erase(this->mountCamera->m_bspFront->m_pointsVec.begin() + selectedFrontPoint);
		selectedFrontPoint = selectedFrontPoint % this->mountCamera->m_bspFront->m_pointsVec.size();
		refreshFront = true;
		break;
	}

	if(refreshPos) {
		this->mountCamera->m_bspPositions->recalculateSpline();
		delete this->pathRenderer;
		this->pathRenderer = new SplineRenderer(this->mountCamera->m_bspPositions);
		this->pathRenderer->setRenderPoints(this->isRenderPathPoints);
	}
	if(refreshFront) {
		this->mountCamera->m_bspFront->recalculateSpline();
		delete this->frontRenderer;
		this->frontRenderer = new SplineRenderer(this->mountCamera->m_bspFront);
		this->frontRenderer->setRenderPoints(this->isRenderFrontPoints);
	}
}

/********************************** EOF ******************************/
