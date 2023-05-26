#include<interpolators.h>

using namespace Eigen;
using namespace std;
using namespace vmath;

/*********************************************************************/
/*                        CubicBezierInterpolator                    */
/*********************************************************************/
CubicBezierInterpolator::CubicBezierInterpolator(const vector<vec3> &inCtrlps)
{
    m_ctrlps = inCtrlps;
    m_nSplines = inCtrlps.size() / 3;
}

CubicBezierInterpolator::CubicBezierInterpolator(const MatrixX3f &inCtrlps)
{
    for (auto ctrlp : inCtrlps.rowwise())
    {
        m_ctrlps.push_back(vec3(ctrlp(0), ctrlp(1), ctrlp(2)));
    }
    m_nSplines = inCtrlps.rows() / 3;
}

vec3 CubicBezierInterpolator::quadraticBezier(const vec3 &A, const vec3 &B, const vec3 &C, const float t)
{
    vec3 D = mix(A, B, t);
    vec3 E = mix(B, C, t);
    return mix(D, E, t);
}

vec3 CubicBezierInterpolator::cubicBezier(const vec3 &A, const vec3 &B, const vec3 &C, const vec3 &D, const float t)
{
    vec3 E = mix(A, B, t);
    vec3 F = mix(B, C, t);
    vec3 G = mix(C, D, t);
    return quadraticBezier(E, F, G, t);
}

float CubicBezierInterpolator::getDistanceOnSpline(float t)
{
    return t * float(m_nSplines);
}

vec3 CubicBezierInterpolator::interpolate(float t)
{
	float ct = clamp(vec1(t), vec1(0.0f), vec1(1.0f))[0];
	float splineLocal = getDistanceOnSpline(ct);
    unsigned indexIntoCtrlps = unsigned(splineLocal - 0.00001f);
	
    vec3 A = m_ctrlps[indexIntoCtrlps * 3 + 0];
    vec3 B = m_ctrlps[indexIntoCtrlps * 3 + 1];
    vec3 C = m_ctrlps[indexIntoCtrlps * 3 + 2];
    vec3 D = m_ctrlps[indexIntoCtrlps * 3 + 3];

    return cubicBezier(A, B, C, D, splineLocal - indexIntoCtrlps);
}

const std::vector<vmath::vec3> &CubicBezierInterpolator::getPoints(void)
{
    throwErr("getPoints() is currently unsupported for CubicBezierInterpolator\n");
    return m_ctrlps;
}

CubicBezierInterpolator::~CubicBezierInterpolator() {}

/*********************************************************************/
/*                          BsplineInterpolator                      */
/*********************************************************************/
BsplineInterpolator::BsplineInterpolator(const vector<vec3> &points)
{
	this->m_pointsVec = points;
	this->m_cubicBezierInterpolator = nullptr;
	recalculateSpline();
}

float BsplineInterpolator::getDistanceOnSpline(const float t)
{
    return m_cubicBezierInterpolator->getDistanceOnSpline(t);
}

vmath::vec3 BsplineInterpolator::interpolate(const float t)
{
    return m_cubicBezierInterpolator->interpolate(t);
}

const std::vector<vmath::vec3> &BsplineInterpolator::getPoints(void)
{
    return m_pointsVec;
}

void BsplineInterpolator::recalculateSpline(void) {
    int row = 0;
	int pointVecSize = this->m_pointsVec.size();
    MatrixX3f m_points(pointVecSize, 3);
    for (auto point : this->m_pointsVec) {
        m_points.row(row)(0) = point[0];
        m_points.row(row)(1) = point[1];
        m_points.row(row)(2) = point[2];
        ++row;
    }
    /* construct the 1-4-1 matrix and get its inverse */
    int nNonEnds = pointVecSize - 2;
    MatrixXf cubicBsplineKernel(nNonEnds, nNonEnds);
    cubicBsplineKernel = MatrixXf::Identity(nNonEnds, nNonEnds) * 4.0f;
    cubicBsplineKernel.block(0, 1, nNonEnds - 1, nNonEnds - 1).triangularView<Upper>().fill(1.0f);
    cubicBsplineKernel.block(0, 2, nNonEnds - 2, nNonEnds - 2).triangularView<Upper>().fill(0.0f);
    cubicBsplineKernel.block(1, 0, nNonEnds - 1, nNonEnds - 1).triangularView<Lower>().fill(1.0f);
    cubicBsplineKernel.block(2, 0, nNonEnds - 2, nNonEnds - 2).triangularView<Lower>().fill(0.0f);
    cubicBsplineKernel = cubicBsplineKernel.inverse();

    /* modify points to use in the 1-4-1 inverse transform equation */
    MatrixX3f tempModifiedPoints(nNonEnds, 3);
    tempModifiedPoints.row(0) = 6.0f * m_points.row(1) - m_points.row(0);
    for (int i = 2; i < nNonEnds; i++)
    {
        tempModifiedPoints.row(i - 1) = 6.0f * m_points.row(i);
    }
    tempModifiedPoints.row(nNonEnds - 1) = 6.0f * m_points.row(pointVecSize - 2) - m_points.row(pointVecSize - 1);

    MatrixX3f m_bspCtrlps(pointVecSize, 3);
    m_bspCtrlps.row(0) = m_points.row(0);
    m_bspCtrlps.block(1, 0, nNonEnds, 3) = cubicBsplineKernel * tempModifiedPoints;
    m_bspCtrlps.row(pointVecSize - 1) = m_points.row(pointVecSize - 1);

	int nCubicCPs = 2 * (m_bspCtrlps.rows() - 1) + m_points.rows();
    MatrixX3f outCtrlps(nCubicCPs, 3);

    int count = 0;
    int i = 0;
    for (; i < m_bspCtrlps.rows() - 1; i++)
    {
        outCtrlps.row(count++) = m_points.row(i); // starts with a knot
        outCtrlps.row(count++) = (2.0f * m_bspCtrlps.row(i) / 3.0f) + (m_bspCtrlps.row(i + 1) / 3.0f);
        outCtrlps.row(count++) = (m_bspCtrlps.row(i) / 3.0f) + (2.0f * m_bspCtrlps.row(i + 1) / 3.0f);
    }
    outCtrlps.row(count++) = m_points.row(i); // ends with a knot

	if(m_cubicBezierInterpolator) {
		delete m_cubicBezierInterpolator;
	}
    m_cubicBezierInterpolator = new CubicBezierInterpolator(outCtrlps);
}

ostream& operator<< (ostream &out, const BsplineInterpolator &t) {
	out<<"Spline = {\n";
	for(int i = 0; i < t.m_pointsVec.size(); i++) {
		out<<"\t"<<t.m_pointsVec[i]<<",\n";
	}
	out<<"}";
	return out;
}

ostream& operator<< (ostream &out, const BsplineInterpolator *t) {
	out<<*t;
	return out;
}

BsplineInterpolator::~BsplineInterpolator()
{	
    if (m_cubicBezierInterpolator)
    {
        delete m_cubicBezierInterpolator;
        m_cubicBezierInterpolator = NULL;
    }
}

/********************************** EOF ******************************/
