#include<splineadjuster.h>
#include<X11/keysym.h>
#include<vmath.h>

using namespace vmath;

SplineAdjuster::SplineAdjuster(BsplineInterpolator* interpolator) {
	this->splineInterpolator = interpolator;
	this->splineRenderer = new SplineRenderer(interpolator);
	this->selectedPoint = 0;
	this->scalingFactor = 1.0f;
	this->isRenderPath = true;
	this->isRenderPoints = true;
}

void SplineAdjuster::keyboardfunc(int key) {
	bool refresh = false;
	switch(key) {
	//Point Select
	case XK_n:
		selectedPoint = (selectedPoint == 0) ? this->splineInterpolator->m_pointsVec.size() - 1 : (selectedPoint - 1) % this->splineInterpolator->m_pointsVec.size();
		break;
	case XK_m:
		selectedPoint = (selectedPoint + 1) % this->splineInterpolator->m_pointsVec.size();
		break;
	//Path Point Move Z
	case XK_i:
		this->splineInterpolator->m_pointsVec[selectedPoint][2] += SPLINE_ADJUSTER_SCALER;
		refresh = true;
		break;
	case XK_k:
		this->splineInterpolator->m_pointsVec[selectedPoint][2] -= SPLINE_ADJUSTER_SCALER;
		refresh = true;
		break;
	//Path Point Move X
	case XK_l:
		this->splineInterpolator->m_pointsVec[selectedPoint][0] += SPLINE_ADJUSTER_SCALER;
		refresh = true;
		break;
	case XK_j:
		this->splineInterpolator->m_pointsVec[selectedPoint][0] -= SPLINE_ADJUSTER_SCALER;
		refresh = true;
		break;
	//Path Point Move Y
	case XK_o:
		this->splineInterpolator->m_pointsVec[selectedPoint][1] += SPLINE_ADJUSTER_SCALER;
		refresh = true;
		break;
	case XK_u:
		this->splineInterpolator->m_pointsVec[selectedPoint][1] -= SPLINE_ADJUSTER_SCALER;
		refresh = true;
		break;
	//Add/Remove Path Points
	case XK_bracketleft:
		this->splineInterpolator->m_pointsVec.insert(this->splineInterpolator->m_pointsVec.begin() + selectedPoint + 1, vec3(0.0f, 0.0f, 0.0f));
		refresh = true;
		break;
	case XK_bracketright:
		this->splineInterpolator->m_pointsVec.erase(this->splineInterpolator->m_pointsVec.begin() + selectedPoint);
		selectedPoint = selectedPoint % this->splineInterpolator->m_pointsVec.size();
		refresh = true;
		break;
	}
	if(refresh) {
		this->splineInterpolator->recalculateSpline();
		delete this->splineRenderer;
		this->splineRenderer = new SplineRenderer(this->splineInterpolator);
		this->splineRenderer->setRenderPoints(this->isRenderPoints);
	}
}

void SplineAdjuster::setRenderPath(bool setting) {
	this->isRenderPath = setting;
}

void SplineAdjuster::setScalingFactor(float scalingFactor) {
	this->scalingFactor = scalingFactor;
}

void SplineAdjuster::setRenderPoints(bool setting) {
	this->splineRenderer->setRenderPoints(setting);
	this->isRenderPoints = setting;
}

void SplineAdjuster::render(const vmath::vec4 linecolor, const vmath::vec4 pointcolor, const vmath::vec4 selectedpointcolor) {
	if(isRenderPath) {
		splineRenderer->render(linecolor, pointcolor, selectedpointcolor, this->selectedPoint, this->scalingFactor);
	}
}

BsplineInterpolator* SplineAdjuster::getSpline() {
	return this->splineInterpolator;
}

SplineAdjuster::~SplineAdjuster() {
	delete this->splineRenderer;
}