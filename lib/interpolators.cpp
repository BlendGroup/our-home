#include "../include/interpolators.h"

using namespace Eigen;
using namespace std;
using namespace vmath;

/*********************************************************************/
/*                        CubicBezierInterpolator                    */
/*********************************************************************/
CubicBezierInterpolator::CubicBezierInterpolator(vector<vec3> &inCtrlps)
{
    m_ctrlps = inCtrlps;
    m_nSplines = inCtrlps.size() / 3;
}

CubicBezierInterpolator::CubicBezierInterpolator(MatrixX3f &inCtrlps)
{
    for (auto ctrlp : inCtrlps.rowwise())
    {
        m_ctrlps.push_back(vec3(ctrlp(0), ctrlp(1), ctrlp(2)));
    }
    m_nSplines = inCtrlps.rows() / 3;
}

vec3 CubicBezierInterpolator::lerp(vec3 &A, vec3 &B, const float t)
{
    vec3 out;
    out[0] = (1.0f - t) * A[0] + t * B[0];
    out[1] = (1.0f - t) * A[1] + t * B[1];
    out[2] = (1.0f - t) * A[2] + t * B[2];
    return out;
}

vec3 CubicBezierInterpolator::quadraticBezier(vec3 &A, vec3 &B, vec3 &C, const float t)
{
    vec3 D = lerp(A, B, t);
    vec3 E = lerp(B, C, t);
    return lerp(D, E, t);
}

vec3 CubicBezierInterpolator::cubicBezier(vec3 &A, vec3 &B, vec3 &C, vec3 &D, const float t)
{
    vec3 E = lerp(A, B, t);
    vec3 F = lerp(B, C, t);
    vec3 G = lerp(C, D, t);
    return quadraticBezier(E, F, G, t);
}

float CubicBezierInterpolator::getDistanceOnSpline(const float t)
{
    return t * float(m_nSplines);
}

vec3 CubicBezierInterpolator::interpolate(const float t)
{
    float splineLocal = getDistanceOnSpline(t);
    int indexIntoCtrlps = int(splineLocal) * 3;

    vec3 A = m_ctrlps[indexIntoCtrlps];
    vec3 B = m_ctrlps[indexIntoCtrlps + 1];
    vec3 C = m_ctrlps[indexIntoCtrlps + 2];
    vec3 D = m_ctrlps[indexIntoCtrlps + 3];

    return cubicBezier(A, B, C, D, splineLocal - int(splineLocal));
}

const std::vector<vmath::vec3> *CubicBezierInterpolator::getPoints(void)
{
    throwErr("getPoints() is currently unsupported for CubicBezierInterpolator\n");
    return NULL;
}

const std::vector<vmath::vec3> *CubicBezierInterpolator::getControlPoints(void)
{
    return &m_ctrlps;
}

CubicBezierInterpolator::~CubicBezierInterpolator() {}

/*********************************************************************/
/*                          BsplineInterpolator                      */
/*********************************************************************/
BsplineInterpolator::BsplineInterpolator(vector<vec3> &points)
{
    /* get points in Eigen matrix form */
    int row = 0;
    m_points = new MatrixX3f(points.size(), 3);
    for (auto point : points)
    {
        m_points->row(row)(0) = point[0];
        m_points->row(row)(1) = point[1];
        m_points->row(row)(2) = point[2];
        ++row;
    }
    m_nPoints = m_points->rows();

    /* copy also the vector form for later submissions to a SplineRenderer
     * (debug-only feature in future)
     */
    m_pointsVec = points;

    /* construct the 1-4-1 matrix and get its inverse */
    int nNonEnds = m_nPoints - 2;
    MatrixXf cubicBsplineKernel(nNonEnds, nNonEnds);
    cubicBsplineKernel = MatrixXf::Identity(nNonEnds, nNonEnds) * 4.0f;
    cubicBsplineKernel.block(0, 1, nNonEnds - 1, nNonEnds - 1).triangularView<Upper>().fill(1.0f);
    cubicBsplineKernel.block(0, 2, nNonEnds - 2, nNonEnds - 2).triangularView<Upper>().fill(0.0f);
    cubicBsplineKernel.block(1, 0, nNonEnds - 1, nNonEnds - 1).triangularView<Lower>().fill(1.0f);
    cubicBsplineKernel.block(2, 0, nNonEnds - 2, nNonEnds - 2).triangularView<Lower>().fill(0.0f);
    cubicBsplineKernel = cubicBsplineKernel.inverse();

    /* modify points to use in the 1-4-1 inverse transform equation */
    MatrixX3f tempModifiedPoints(nNonEnds, 3);
    tempModifiedPoints.row(0) = 6.0f * m_points->row(1) - m_points->row(0);
    for (int i = 2; i < nNonEnds; i++)
    {
        tempModifiedPoints.row(i - 1) = 6.0f * m_points->row(i);
    }
    tempModifiedPoints.row(nNonEnds - 1) = 6.0f * m_points->row(m_nPoints - 2) - m_points->row(m_nPoints - 1);

    /* apply inverse transform equation to compute bspline control points */
    m_bspCtrlps = new MatrixX3f(m_nPoints, 3);
    m_bspCtrlps->row(0) = m_points->row(0);
    m_bspCtrlps->block(1, 0, nNonEnds, 3) = cubicBsplineKernel * tempModifiedPoints;
    m_bspCtrlps->row(m_nPoints - 1) = m_points->row(m_nPoints - 1);

    /* copy bspline ctrlps to a vector form for later submissions to a SplineRenderer
     * (debug-only feature in future)
     */
    for(auto ctrlp: m_bspCtrlps->rowwise())
    {
        m_bspCtrlpsVec.push_back(vec3(ctrlp(0), ctrlp(1), ctrlp(2)));
    }

    /* get subdivided cubic bezier control points from bspline control points */
    MatrixX3f cubicBezierCtrlps = getCubicBezierControlPoints();

    /* optain a cubic interpolator from subdivided bezier control points */
    m_cubicBezierInterpolator = new CubicBezierInterpolator(cubicBezierCtrlps);
}

MatrixX3f BsplineInterpolator::getCubicBezierControlPoints(void)
{
    int nCubicCPs = 2 * (m_bspCtrlps->rows() - 1) + m_points->rows();
    MatrixX3f outCtrlps(nCubicCPs, 3);

    int count = 0;
    int i = 0;
    for (; i < m_bspCtrlps->rows() - 1; i++)
    {
        outCtrlps.row(count++) = m_points->row(i); // starts with a knot
        outCtrlps.row(count++) = (2.0f * m_bspCtrlps->row(i) / 3.0f) + (m_bspCtrlps->row(i + 1) / 3.0f);
        outCtrlps.row(count++) = (m_bspCtrlps->row(i) / 3.0f) + (2.0f * m_bspCtrlps->row(i + 1) / 3.0f);
    }
    outCtrlps.row(count++) = m_points->row(i); // ends with a knot
    return outCtrlps;
}

float BsplineInterpolator::getDistanceOnSpline(const float t)
{
    return m_cubicBezierInterpolator->getDistanceOnSpline(t);
}

vmath::vec3 BsplineInterpolator::interpolate(const float t)
{
    return m_cubicBezierInterpolator->interpolate(t);
}

const std::vector<vmath::vec3> *BsplineInterpolator::getPoints(void)
{
    return &m_pointsVec;
}

const std::vector<vmath::vec3> *BsplineInterpolator::getControlPoints(void)
{
    return &m_bspCtrlpsVec;
}

BsplineInterpolator::~BsplineInterpolator()
{
    if (m_cubicBezierInterpolator)
    {
        delete m_cubicBezierInterpolator;
        m_cubicBezierInterpolator = NULL;
    }
    if (m_bspCtrlps)
    {
        delete m_bspCtrlps;
        m_bspCtrlps = NULL;
    }
    if (m_points)
    {
        delete m_points;
        m_points = NULL;
    }
}

/********************************** EOF ******************************/
