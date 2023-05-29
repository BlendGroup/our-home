#ifndef __SPLINE_ADJUSTER__
#define __SPLINE_ADJUSTER__

#include<interpolators.h>
#include<splinerenderer.h>

#ifndef SPLINE_ADJUSTER_SCALER
#define SPLINE_ADJUSTER_SCALER 0.01f
#endif

#define BLUE_CYAN_COLOR vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(0.0f, 1.0f, 1.0f, 1.0f)
#define GREEN_YELLOW_COLOR vec4(0.0f, 0.5f, 0.0f, 1.0f), vec4(0.0f, 1.0f, 0.0f, 1.0f), vec4(1.0f, 1.0f, 0.0f, 1.0f)
#define RED_PINK_COLOR vec4(0.0f, 0.0f, 0.5f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 0.0f, 1.0f, 1.0f)

class SplineAdjuster {
private:
	BsplineInterpolator* splineInterpolator;
	SplineRenderer* splineRenderer;
	bool isRenderPath;
	bool isRenderPoints;
	int selectedPoint;
	float scalingFactor;
public:
	SplineAdjuster(BsplineInterpolator* spline);
	void setRenderPath(bool setting);
	void setRenderPoints(bool setting);
	void setScalingFactor(float scalingFactor);
	void keyboardfunc(int key);
	void render(const vmath::vec4 linecolor, const vmath::vec4 pointcolor, const vmath::vec4 selectedpointcolor);
	BsplineInterpolator* getSpline();
	~SplineAdjuster();
};

#endif