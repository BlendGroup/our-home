#ifndef __INTERPOLATORS_H__
#define __INTERPOLATORS_H__

#ifndef DEBUG
#define EIGEN_NO_DEBUG
#endif // DEBUG

#include <iostream>
#include <vector>
#include "vmath.h"
#include "Eigen/LU"
#include "errorlog.h"

class SplineInterpolator
{
public:
    virtual vmath::vec3 interpolate(const float t) = 0;
    virtual const std::vector<vmath::vec3> *getPoints(void) = 0;
    virtual const std::vector<vmath::vec3> *getControlPoints(void) = 0;
};

class CubicBezierInterpolator: public SplineInterpolator
{
private:
    std::vector<vmath::vec3> m_ctrlps;
    int m_nSplines;

    vmath::vec3 lerp(const vmath::vec3 &A, const vmath::vec3 &B, const float t);
    vmath::vec3 quadraticBezier(const vmath::vec3 &A, const vmath::vec3 &B, const vmath::vec3 &C, const float t);
    vmath::vec3 cubicBezier(const vmath::vec3 &A, const vmath::vec3 &B, const vmath::vec3 &C, const vmath::vec3 &D, const float t);
public:
    CubicBezierInterpolator(const std::vector<vmath::vec3> &inCtrlps);
    CubicBezierInterpolator(const Eigen::MatrixX3f &inCtrlps);
    ~CubicBezierInterpolator();
    float getDistanceOnSpline(const float t);
    vmath::vec3 interpolate(const float t) override;
    const std::vector<vmath::vec3> *getPoints(void) override;
    const std::vector<vmath::vec3> *getControlPoints(void) override;
};

class BsplineInterpolator: public SplineInterpolator
{
private:
    Eigen::MatrixX3f *m_points;
    Eigen::MatrixX3f *m_bspCtrlps;
    std::vector<vmath::vec3> m_pointsVec;
    std::vector<vmath::vec3> m_bspCtrlpsVec;
    CubicBezierInterpolator *m_cubicBezierInterpolator;
    int m_nPoints;

    Eigen::MatrixX3f getCubicBezierControlPoints(void);
public:
    BsplineInterpolator(const std::vector<vmath::vec3> &points);
    ~BsplineInterpolator();
    float getDistanceOnSpline(const float t);
    vmath::vec3 interpolate(const float t) override;
    const std::vector<vmath::vec3> *getPoints(void) override;
    const std::vector<vmath::vec3> *getControlPoints(void) override;
};

#endif // __INTERPOLATORS_H__
