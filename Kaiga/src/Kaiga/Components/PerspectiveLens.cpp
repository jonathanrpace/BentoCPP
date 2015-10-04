#include "PerspectiveLens.h"

#include <glm/gtc/matrix_transform.hpp>

Kaiga::PerspectiveLens::PerspectiveLens() :
	m_aspectRatio(1.0f),
	m_near(0.001f),
	m_far(10.0f),
	m_verticalFOV(45.0f * 0.0174f),
	m_matrix(),
	m_matrixIsInvalid(false)
{
	
}

float Kaiga::PerspectiveLens::GetAspectRatio()
{
	return m_aspectRatio;
}

void Kaiga::PerspectiveLens::SetAspectRatio(float _value)
{
	m_aspectRatio = _value;
	m_matrixIsInvalid = true;
}

const glm::mat4& Kaiga::PerspectiveLens::GetMatrix()
{
	if ( m_matrixIsInvalid )
	{
		m_matrix = glm::perspective(m_verticalFOV, m_aspectRatio, m_near, m_far);
		m_matrixIsInvalid = false;
	}

	return m_matrix;
}

float Kaiga::PerspectiveLens::GetVerticalFOV()
{
	return m_verticalFOV;
}

void Kaiga::PerspectiveLens::SetVerticalFOV(float _value)
{
	m_verticalFOV = _value;
	m_matrixIsInvalid = true;
}

float Kaiga::PerspectiveLens::GetHorizontalFOV()
{
	return m_verticalFOV * m_aspectRatio;
}

void Kaiga::PerspectiveLens::SetHorizontalFOV(float _value)
{
	m_verticalFOV = _value / m_aspectRatio;
	m_matrixIsInvalid = true;
}

float Kaiga::PerspectiveLens::GetNear()
{
	return m_near;
}

void Kaiga::PerspectiveLens::SetNear(float _value)
{
	m_near = _value;
	m_matrixIsInvalid = true;
}

float Kaiga::PerspectiveLens::GetFar()
{
	return m_far;
}

void Kaiga::PerspectiveLens::SetFar(float _value)
{
	m_far = _value;
	m_matrixIsInvalid = true;
}
