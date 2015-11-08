#pragma once

#include <bento.h>

namespace bento
{
	struct ILens
	{
		virtual float GetAspectRatio() = 0;
		virtual void SetAspectRatio(float _value) = 0;
		virtual const mat4& GetMatrix() = 0;
	};
}