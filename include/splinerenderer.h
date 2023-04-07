#ifndef __SPLINE_RENDERER_H__
#define __SPLINE_RENDERER_H__

#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include "../include/vmath.h"
#include "../include/glshaderloader.h"
#include "../include/interpolators.h"

class SplineRenderer
{
private:
    SplineInterpolator *m_interpolator;
    glshaderprogram *m_program;
    GLuint m_vaoSpline, m_vboSpline;
    GLuint m_vaoPoint, m_vboPoint;
    GLuint m_vaoCtrlPoly, m_vboCtrlPoly;
    const std::vector<vmath::vec3> *m_points, *m_ctrlps;
    int m_nAllPositions;
    float m_linspace;

    void loadGeometry(void);

public:
    SplineRenderer(SplineInterpolator *interpolator, const float linspace = 0.01f);
    ~SplineRenderer();
    void render(vmath::mat4 &viewMatrix, vmath::mat4 &projMatrix, const vmath::vec4 &color);
    void renderPoints(vmath::mat4 &viewMatrix, vmath::mat4 &projMatrix);
    void renderControlPoints(vmath::mat4 &viewMatrix, vmath::mat4 &projMatrix);
    void renderControlPolygon(vmath::mat4 &viewMatrix, vmath::mat4 &projMatrix, const vmath::vec4 &color);
};

#endif // __SPLINE_RENDERER_H__
