#include<splinerenderer.h>

using namespace std;
using namespace vmath;

/*********************************************************************/
/*                              SplineRenderer                       */
/*********************************************************************/
SplineRenderer::SplineRenderer(SplineInterpolator *interpolator, const float linspace)
	: m_interpolator(interpolator),
	  m_nAllPositions(0),
	  m_linspace(linspace),
	  m_isRenderPoints(false),
	  m_isRenderCtrlps(false),
	  m_isRenderCtrlPoly(false)
{
	/* create vaos for spline data */
	glCreateVertexArrays(1, &m_vaoSpline);
	glCreateVertexArrays(1, &m_vaoPoint);
	glCreateVertexArrays(1, &m_vaoCtrlPoly);

	/* create vbos for spline data */
	glCreateBuffers(1, &m_vboSpline);
	glCreateBuffers(1, &m_vboPoint);
	glCreateBuffers(1, &m_vboCtrlPoly);

	/* create program for rendering a spline */
	m_program = new glshaderprogram({"shaders/spline.vert",
									 "shaders/spline.frag"});

	/* get user-specified spline points from the interpolator */
	m_points = m_interpolator->getPoints();
	m_ctrlps = m_interpolator->getControlPoints();

	/* load spline data into opengl pipeline */
	loadGeometry();
}

void SplineRenderer::loadGeometry(void)
{
	/* interpolate through the entire spline and load into pipeline */
	vector<vec3> allPositions;
	for (float t = 0.0f; t < 1.0f; t += m_linspace)
	{
		vec3 value = m_interpolator->interpolate(t);
		allPositions.push_back(vec3(value[0], value[1], value[2]));
		++m_nAllPositions;
	}

	glBindVertexArray(m_vaoSpline);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboSpline);

	/** !!! BE CAREFUL WHILE REFACTORING FOR vec3 !!! **/
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * allPositions.size(), allPositions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	/* load control points into pipeline */
	glBindVertexArray(m_vaoCtrlPoly);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboCtrlPoly);

	/** !!! BE CAREFUL WHILE REFACTORING FOR vec3 !!! **/
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * m_ctrlps->size(), m_ctrlps->data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	/* load a cube to mark points into pipeline */
	const float verts[] = {
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,

		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f};

	glBindVertexArray(m_vaoPoint);
	glBindBuffer(GL_ARRAY_BUFFER, m_vboPoint);

	/** !!! BE CAREFUL WHILE REFACTORING FOR vec3 !!! **/
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

void SplineRenderer::render(const vec4 color) const
{
	m_program->use();
	glUniformMatrix4fv(0, 1, GL_FALSE, programglobal::perspective * programglobal::currentCamera->matrix());
	glUniform4fv(1, 1, color);
	glUniform1i(2, 0); // isPoint = false
	glBindVertexArray(m_vaoSpline);
	glDrawArrays(GL_LINE_STRIP, 0, m_nAllPositions);

	if(m_isRenderPoints)
	{
		glUniform1i(2, 1); // isPoint = true
		glBindVertexArray(m_vaoPoint);

		/** !!! BE CAREFUL WHILE REFACTORING FOR vec3 !!! **/
		for (vec3 point : *m_points)
		{
			glUniformMatrix4fv(0, 1, GL_FALSE,
							programglobal::perspective * programglobal::currentCamera->matrix() *
								translate(point[0], point[1], point[2]) *
								scale(0.1f, 0.1f, 0.1f));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	if(m_isRenderCtrlps)
	{
		glUniform1i(2, 1); // isPoint = true
		glBindVertexArray(m_vaoPoint);

		/** !!! BE CAREFUL WHILE REFACTORING FOR vec3 !!! **/
		for (vec3 ctrlp : *m_ctrlps)
		{
			glUniformMatrix4fv(0, 1, GL_FALSE,
							programglobal::perspective * programglobal::currentCamera->matrix() *
								translate(ctrlp[0], ctrlp[1], ctrlp[2]) *
								scale(0.1f, 0.1f, 0.1f));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	if(m_isRenderCtrlPoly)
	{
		/* TODO: this feature has a bug, find and fix it */
	
		// glUniformMatrix4fv(0, 1, GL_FALSE, viewMatrix * projMatrix);
		// glUniform4fv(1, 1, vec4(0.8f, 0.2f, 0.0f, 1.0f));
		// glUniform1i(2, 0); // isPoint = false
		// glBindVertexArray(m_vaoCtrlPoly);
		// glDrawArrays(GL_LINES, 0, m_ctrlps->size());
	}
}

void SplineRenderer::setRenderPoints(bool setting)
{
	m_isRenderPoints = setting;
}

void SplineRenderer::setRenderControlPoints(bool setting)
{
	m_isRenderCtrlps = setting;
}

void SplineRenderer::setRenderControlPolygon(bool setting)
{
	m_isRenderCtrlPoly = setting;
}

SplineRenderer::~SplineRenderer()
{
	if (m_vboCtrlPoly)
	{
		glDeleteBuffers(1, &m_vboCtrlPoly);
		m_vboCtrlPoly = 0;
	}
	if (m_vaoCtrlPoly)
	{
		glDeleteVertexArrays(1, &m_vaoCtrlPoly);
		m_vaoCtrlPoly = 0;
	}
	if (m_vboPoint)
	{
		glDeleteBuffers(1, &m_vboPoint);
		m_vboPoint = 0;
	}
	if (m_vaoPoint)
	{
		glDeleteVertexArrays(1, &m_vaoPoint);
		m_vaoPoint = 0;
	}
	if (m_vboSpline)
	{
		glDeleteBuffers(1, &m_vboSpline);
		m_vboSpline = 0;
	}
	if (m_vaoSpline)
	{
		glDeleteVertexArrays(1, &m_vaoSpline);
		m_vaoSpline = 0;
	}
	if (m_program)
	{
		delete m_program;
		m_program = NULL;
	}
}

/********************************** EOF ******************************/
