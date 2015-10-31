#pragma once

#include <glm.h>

#include <bento/core/IComponent.h>

namespace bento
{
	class ILens : 
		public bento::IComponent
	{
	public:
		virtual float GetAspectRatio() = 0;
		virtual void SetAspectRatio(float _value) = 0;
		virtual const mat4& GetMatrix() = 0;
	};
}