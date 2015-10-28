#pragma once

#include <typeinfo>

#include <Ramen\Core\SharedObject.h>
#include <Kaiga\Core\ILens.h>

namespace Kaiga
{
	class PerspectiveLens 
		: public Ramen::SharedObject<PerspectiveLens>
		, public ILens
	{
	public:
		PerspectiveLens();

		// From ISceneObject
		virtual const std::type_info & typeInfo() override;

		// From ILens
		virtual float GetAspectRatio() override;
		virtual void SetAspectRatio(float _value) override;
		virtual const mat4& GetMatrix() override;

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
		mat4 m_matrix;
		bool m_matrixIsInvalid;
	};
}