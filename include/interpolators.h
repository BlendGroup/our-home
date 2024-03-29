#ifndef __INTERPOLATORS_H__
#define __INTERPOLATORS_H__

#ifndef DEBUG
#define EIGEN_NO_DEBUG
#endif // DEBUG

#include<iostream>
#include<vector>
#include<vmath.h>
#include<Eigen/LU>
#include<errorlog.h>

class SplineInterpolator
{
public:
    virtual vmath::vec3 interpolate(float t) = 0;
    virtual const std::vector<vmath::vec3> &getPoints(void) = 0;
};

class CubicBezierInterpolator: public SplineInterpolator
{
private:
    std::vector<vmath::vec3> m_ctrlps;
    int m_nSplines;

    vmath::vec3 quadraticBezier(const vmath::vec3 &A, const vmath::vec3 &B, const vmath::vec3 &C, const float t);
    vmath::vec3 cubicBezier(const vmath::vec3 &A, const vmath::vec3 &B, const vmath::vec3 &C, const vmath::vec3 &D, const float t);
public:
    CubicBezierInterpolator(const std::vector<vmath::vec3> &inCtrlps);
    CubicBezierInterpolator(const Eigen::MatrixX3f &inCtrlps);
    ~CubicBezierInterpolator();
    float getDistanceOnSpline(const float t);
    vmath::vec3 interpolate(const float t) override;
    const std::vector<vmath::vec3> &getPoints(void) override;
};

class BsplineInterpolator: public SplineInterpolator
{
private:
    std::vector<vmath::vec3> m_pointsVec;
    CubicBezierInterpolator *m_cubicBezierInterpolator;
	friend class sceneCameraRig;
	friend class SplineAdjuster;
public:
    BsplineInterpolator(const std::vector<vmath::vec3> &points);
    ~BsplineInterpolator();
    float getDistanceOnSpline(const float t);
	void recalculateSpline(void);
    vmath::vec3 interpolate(const float t) override;
    const std::vector<vmath::vec3> &getPoints(void) override;
	friend std::ostream& operator<< (std::ostream &out, const BsplineInterpolator *t);
	friend std::ostream& operator<< (std::ostream &out, const BsplineInterpolator &t);
};

#endif // __INTERPOLATORS_H__
