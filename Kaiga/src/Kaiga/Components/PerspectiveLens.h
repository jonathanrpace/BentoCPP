#pragma once

#include <Ramen/Components/ComponentBase.h>

#include <Kaiga/Components/ILens.h>

namespace Kaiga
{
	class PerspectiveLens 
		: public Ramen::ComponentBase<PerspectiveLens>
		, public ILens
	{
	public:
		PerspectiveLens();

		// From ILens
		virtual float GetAspectRatio() override;
		virtual void SetAspectRatio(float _value) override;
		virtual const glm::mat4& GetMatrix() override;

		float GetVerticalFOV();
		void SetVerticalFOV(float _value);
		float GetHorizontalFOV();
		void SetHorizontalFOV(float _value);
		float GetNear();
		void SetNear(float _value);
		float GetFar();
		void SetFar(float _value);

	private:
		float m_aspectRatio;
		float m_verticalFOV;
		float m_near;
		float m_far;
		glm::mat4 m_matrix;
		bool m_matrixIsInvalid;
	};
}