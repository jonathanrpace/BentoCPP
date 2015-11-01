#pragma once

#include <bento/core/Component.h>

namespace bento
{
	struct IGeometry : Component
	{
		IGeometry(std::string _name = "Geometry");

		virtual void Bind() = 0;
		virtual void Draw() = 0;
	};
}