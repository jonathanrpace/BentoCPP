#pragma once

#include <bento/core/Component.h>

namespace bento
{
	struct ILens : Component
	{
		ILens(std::string _name = "ILens");

		virtual float GetAspectRatio() = 0;
		virtual void SetAspectRatio(float _value) = 0;
		virtual const mat4& GetMatrix() = 0;
	};
}