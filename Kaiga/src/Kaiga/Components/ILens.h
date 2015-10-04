#pragma once

#include <GLM/glm.hpp>

namespace Kaiga
{
	class ILens
	{
	public:
		virtual float GetAspectRatio() = 0;
		virtual void SetAspectRatio(float _value) = 0;
		virtual const glm::mat4& GetMatrix() = 0;
	};
}