#ifndef __SPLINE_RENDERER_H__
#define __SPLINE_RENDERER_H__

#include<vector>
#include<GL/glew.h>
#include<GL/gl.h>
#include<vmath.h>
#include<glshaderloader.h>
#include<interpolators.h>
#include<camera.h>
#include<global.h>

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
    bool m_isRenderPoints, m_isRenderCtrlps, m_isRenderCtrlPoly;

    void loadGeometry(void);

public:
    SplineRenderer(SplineInterpolator *interpolator, const float linspace = 0.01f);
    ~SplineRenderer();
    void render(const camera* &camera, const vmath::vec4 &color) const;
    void setRenderPoints(bool setting);
    void setRenderControlPoints(bool setting);
    void setRenderControlPolygon(bool setting);
};

#endif // __SPLINE_RENDERER_H__
