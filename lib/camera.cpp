#include "../include/camera.h"

using namespace std;
using namespace vmath;

/*********************************************************************/
/*                              SceneCamera                          */
/*********************************************************************/
SceneCamera::SceneCamera(const PathDescriptor *pdesc)
{
    if (pdesc->positionKeyFrames.size() <= 0)
    {
        throwErr("SceneCamera cannot take empty positionKeyFrames vector");
        return;
    }
    if (pdesc->frontKeyFrames.size() <= 0)
    {
        throwErr("SceneCamera cannot take empty frontKeyFrames vector");
        return;
    }

    m_bspPositions = new BsplineInterpolator(pdesc->positionKeyFrames);
    m_bspFront = new BsplineInterpolator(pdesc->frontKeyFrames);
    m_pdesc = pdesc;
}

float SceneCamera::getDistanceOnSpline(const float t) const
{
    return m_bspPositions->getDistanceOnSpline(t);
}

mat4 SceneCamera::matrix(const float t) const
{
    static const vec3 up = vec3(0.0f, 1.0f, 0.0f);
    vec3 eye = m_bspPositions->interpolate(t);
    vec3 center = m_bspFront->interpolate(t);
    return lookat(eye, center, up);
}

SceneCamera::~SceneCamera()
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

/********************************** EOF ******************************/
