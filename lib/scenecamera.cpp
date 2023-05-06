#include<scenecamera.h>

using namespace std;
using namespace vmath;

/*********************************************************************/
/*                              SceneCamera                          */
/*********************************************************************/
sceneCamera::sceneCamera(const vector<vec3> &positionKeyFrames, const vector<vec3> &frontKeyFrames)
{
    if (positionKeyFrames.size() <= 0)
    {
        throwErr("SceneCamera cannot take empty positionKeyFrames vector");
        return;
    }
    if (frontKeyFrames.size() <= 0)
    {
        throwErr("SceneCamera cannot take empty frontKeyFrames vector");
        return;
    }

	this->frontKeyFrames = frontKeyFrames;
	this->positionKeyFrames = positionKeyFrames;
    m_bspPositions = new BsplineInterpolator(positionKeyFrames);
    m_bspFront = new BsplineInterpolator(frontKeyFrames);
	this->t = 0.0f;
}

void sceneCamera::updateT(float speed) {
	this->t = std::min(this->t + speed, 1.0f);
}

void sceneCamera::resetT() {
	this->t = 0.0f;
}

float sceneCamera::getDistanceOnSpline() const
{
    return m_bspPositions->getDistanceOnSpline(t);
}

mat4 sceneCamera::matrix() const
{
    static const vec3 up = vec3(0.0f, 1.0f, 0.0f);
    vec3 eye = m_bspPositions->interpolate(t);
    vec3 center = m_bspFront->interpolate(t);
    return lookat(eye, center, up);
}

vec3 sceneCamera::position() const {
	return this->m_bspPositions->interpolate(t);
}

sceneCamera::~sceneCamera()
{
    if (m_bspFront)
    {
        delete m_bspFront;
        m_bspFront = NULL;
    }
    if (m_bspPositions)
    {
        delete m_bspPositions;
        m_bspPositions = NULL;
    }
}

ostream& operator<<(ostream &out, const sceneCamera &t) {
	out<<"Position: {\n";
	for(vec3 pos : t.positionKeyFrames) {
		out<<"\t"<<pos<<"\n";
	}
	out<<"}\n";
	out<<"Front: {\n";
	for(vec3 front : t.frontKeyFrames) {
		out<<"\t"<<front<<"\n";
	}
	out<<"}";
	return out;
}

ostream& operator<<(ostream &out, const sceneCamera *t) {
	out<<*t;
	return out;
}

/********************************** EOF ******************************/
