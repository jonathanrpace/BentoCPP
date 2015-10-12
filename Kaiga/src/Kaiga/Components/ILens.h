#pragma once

#include <glm.h>
#include <ramen.h>

namespace Kaiga
{
	class ILens : 
		public Ramen::IComponent
	{
	public:
		virtual float GetAspectRatio() = 0;
		virtual void SetAspectRatio(float _value) = 0;
		virtual const mat4& GetMatrix() = 0;
	};
}