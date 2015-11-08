#include "PerspectiveLens.h"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace bento
{
	PerspectiveLens::PerspectiveLens(std::string _name) 
		: Component(_name, typeid(PerspectiveLens))
		, m_aspectRatio(1.0f)
		, m_near(0.001f)
		, m_far(10.0f)
		, m_verticalFOV(45.0f * 0.0174f)
		, m_matrix()
		, m_matrixIsInvalid(true)
	{

	}

	float PerspectiveLens::GetAspectRatio()
	{
		return m_aspectRatio;
	}

	void PerspectiveLens::SetAspectRatio(float _value)
	{
		_value = std::isnan(_value) ? 1.0f : _value;

		m_aspectRatio = _value;
		m_matrixIsInvalid = true;
	}

	const mat4& PerspectiveLens::GetMatrix()
	{
		if (m_matrixIsInvalid)
		{
			m_matrix = glm::perspective(m_verticalFOV, m_aspectRatio, m_near, m_far);
			m_matrixIsInvalid = false;
		}

		return m_matrix;
	}

	float PerspectiveLens::GetVerticalFOV()
	{
		return m_verticalFOV;
	}

	void PerspectiveLens::SetVerticalFOV(float _value)
	{
		m_verticalFOV = _value;
		m_matrixIsInvalid = true;
	}

	float PerspectiveLens::GetHorizontalFOV()
	{
		return m_verticalFOV * m_aspectRatio;
	}

	void PerspectiveLens::SetHorizontalFOV(float _value)
	{
		m_verticalFOV = _value / m_aspectRatio;
		m_matrixIsInvalid = true;
	}

	float PerspectiveLens::GetNear()
	{
		return m_near;
	}

	void PerspectiveLens::SetNear(float _value)
	{
		m_near = _value;
		m_matrixIsInvalid = true;
	}

	float PerspectiveLens::GetFar()
	{
		return m_far;
	}

	void PerspectiveLens::SetFar(float _value)
	{
		m_far = _value;
		m_matrixIsInvalid = true;
	}
}