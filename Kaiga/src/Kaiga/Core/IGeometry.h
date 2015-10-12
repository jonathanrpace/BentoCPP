#pragma once

#include <ramen.h>

namespace Kaiga
{
	class IGeometry 
		: public Ramen::IComponent
	{
		virtual void Bind() = 0;
		virtual void Draw() = 0;
	};
}